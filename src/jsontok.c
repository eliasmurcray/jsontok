#include "jsontok.h"

static struct JsonToken *jsontok_parse_value(const char **json_string, enum JsonError *error);

const char *jsontok_strerror(enum JsonError error) {
  switch (error) {
    case JSON_ENOERR:
      return "No error";
    case JSON_EFMT:
      return "Invalid format";
    case JSON_ETYPE:
      return "Invalid type";
    case JSON_ENOMEM:
      return "Out of memory";
    default:
      return "Unknown error";
  }
}

void jsontok_free(struct JsonToken *token) {
  if (token == NULL) return;
  switch(token->type) {
    case JSON_ARRAY: {
      size_t i;
      for (i = 0; i < token->as_array->length; i++) {
        jsontok_free(token->as_array->elements[i]);
      }
      break;
    }
    case JSON_OBJECT: {
      size_t i;
      for (i = 0; i < token->as_object->count; i++) {
        free(token->as_object->entries[i]->key);
        jsontok_free(token->as_object->entries[i]->value);
      }
      break;
    }
    case JSON_WRAPPED_OBJECT:
    case JSON_WRAPPED_ARRAY:
    case JSON_STRING:
      free(token->as_string);
      break;
    default:
      break;
  }
  free(token);
}

struct JsonToken *jsontok_get(struct JsonObject *object, const char *key) {
  if (!key) {
    return NULL;
  }
  size_t length = strlen(key);
  if (length == 0) {
    return NULL;
  }
  /**
   * Hashes are not used as the number of entries will never exceed the
   * bucket count, therefore complexities are the same with or without
   * hashes (no collisions).
   */
  size_t i;
  for (i = 0; i < object->count; i++) {
    if (strncmp(object->entries[i]->key, key, length) == 0) {
      return object->entries[i]->value;
    }
  }
  return NULL;
}

static char *jsontok_parse_string(const char **json_string, enum JsonError *error) {
  char *ptr = (char *)(*json_string + 1);
  ptr ++;
  size_t length = 1;
  while (*ptr != '"') {
    if (*ptr == '\0') {
      *error = JSON_EFMT;
      return NULL;
    }
    ptr += (*ptr == '\\') + 1;
    length ++;
  }
  char *substr = malloc(length);
  if (!substr) {
    *error = JSON_ENOMEM;
    return NULL;
  }
  size_t i = 0;
  char *nptr = (char *)(*json_string + 1);
  for (; nptr != ptr; nptr++, i++) {
    if (*nptr != '\\') {
      substr[i] = *nptr;
      continue;
    }
    nptr ++;
    switch (*nptr) {
      case 'b':
        substr[i] = '\b';
        break;
      case 'f':
        substr[i] = '\f';
        break;
      case 'n':
        substr[i] = '\n';
        break;
      case 'r':
        substr[i] = '\r';
        break;
      case 't':
        substr[i] = '\t';
        break;
      case '"':
        substr[i] = '"';
        break;
      case '\\':
        substr[i] = '\\';
        break;
      default:
        free(substr);
        *error = JSON_EFMT;
        return NULL;
    }
  }
  substr[i] = '\0';
  *json_string = ptr + 1;
  return substr;
}

static struct JsonToken *jsontok_parse_number_token(const char **json_string, enum JsonError *error) {
  char *ptr = (char *)(*json_string + 1);
  unsigned char needs_digit = **json_string == '-';
  unsigned char decimal = 1;
  while (1) {
    char c = *ptr++;
    if (c >= '0' && c <= '9') continue;
    if (needs_digit) {
      *error = JSON_EFMT;
      return NULL;
    }
    if (decimal && c == '.') {
      decimal = 0;
      continue;
    }
    break;
  }
  ptr --;
  size_t length = ptr - *json_string + 1;
  char *substr = malloc(length);
  if (!substr) {
    *error = JSON_ENOMEM;
    return NULL;
  }
  strncpy(substr, *json_string, length - 1);
  substr[length] = '\0';
  struct JsonToken *token = malloc(sizeof(struct JsonToken));
  if (!token) {
    *error = JSON_ENOMEM;
    free(substr);
    return NULL;
  }
  errno = 0;
  char *endptr = NULL;
  if (!decimal) {
    token->type = JSON_DOUBLE;
    double d = strtod(substr, &endptr);
    if (errno || *endptr != '\0') {
      *error = JSON_EFMT;
      free(substr);
      free(token);
      return NULL;
    }
    free(substr);
    token->as_double = d;
  } else {
    token->type = JSON_LONG;
    long l = strtol(substr, &endptr, 10);
    if (errno || *endptr != '\0') {
      *error = JSON_EFMT;
      free(substr);
      free(token);
      return NULL;
    }
    free(substr);
    token->as_long = l;
  }
  *json_string = ptr;
  return token;
}

/**
 * @brief Wraps a JSON object in a string for iterative parsing.
 *
 * Both `jsontok_wrap_array` and `jsontok_wrap_object` enable iterative parsing by
 * requiring the user to unwrap subobjects and subarrays when they are needed.
 *
 * @param json_string A char pointer to the beginning of the JSON object.
 * @param error A pointer to a char pointer where an error message will be set on failure.
 * @return A char pointer to the wrapped JSON object string. On error, NULL is returned
 *         and `error` is set.
 */
static char *jsontok_wrap_object(const char **json_string, enum JsonError *error) {
  char *ptr = (char *)(*json_string + 1);
  size_t counter = 1;
  while (counter > 1 || *ptr != '}') {
    if (*ptr == '\0') {
      *error = JSON_EFMT;
      return NULL;
    }
    if (*ptr == '{') counter ++;
    else if (*ptr == '}') counter --;
    ptr += (*ptr == '\\') + 1;
  }
  size_t length = ptr - *json_string + 2;
  char *substr = malloc(length);
  if (!substr) {
    *error = JSON_ENOMEM;
    return NULL;
  }
  strncpy(substr, *json_string, length - 1);
  substr[length] = '\0';
  *json_string = ptr + 1;
  return substr;
}

/**
 * @brief Wraps a JSON array in a string for iterative parsing.
 *
 * Both `jsontok_wrap_array` and `jsontok_wrap_object` enable iterative parsing by
 * requiring the user to unwrap subobjects and subarrays when they are needed.
 *
 * @param json_string A pointer to a char pointer to the beginning of the JSON array.
 * @param error A pointer to a char pointer where an error message will be set on failure.
 * @return A char pointer to the wrapped JSON array string. On error, NULL is returned
 *         and the error parameter is set with an appropriate message.
 */
static char *jsontok_wrap_array(const char **json_string, enum JsonError *error) {
  char *ptr = (char *)(*json_string + 1);
  size_t counter = 1;
  while (counter > 1 || *ptr != ']') {
    if (*ptr == '\0') {
      *error = JSON_EFMT;
      return NULL;
    }
    if (*ptr == '[') counter ++;
    else if (*ptr == ']') counter --;
    ptr += (*ptr == '\\') + 1;
  }
  size_t length = ptr - *json_string + 2;
  char *substr = malloc(length);
  if (!substr) {
    *error = JSON_EFMT;
    return NULL;
  }
  strncpy(substr, *json_string, length - 1);
  substr[length] = '\0';
  *json_string = ptr + 1;
  return substr;
}

/**
 * To fill an array or object in one pass, data is temporarily stored in a linked list
 * and expanded into a malloc'd pointer once traversal is complete, avoiding realloc.
 */
struct Node {
  void *value;
  struct Node *next;
};

static void free_list_object(struct Node *n) {
  while (n) {
    struct Node* t = n;
    n = n->next;
    if (t->value == NULL) continue;
    struct JsonEntry *entry = (struct JsonEntry *)t->value;
    free(entry->key);
    jsontok_free(entry->value);
    free(entry);
    free(t);
  }
}

static void free_list_array(struct Node *n) {
  while (n) {
    struct Node* t = n;
    n = n->next;
    jsontok_free((struct JsonToken *)t->value);
    free(t);
  }
}

static struct JsonObject *jsontok_parse_object(const char *json_string, enum JsonError *error) {
  struct Node *head = malloc(sizeof(struct Node));
  if (!head) {
    *error = JSON_ENOMEM;
    return NULL;
  }
  head->next = NULL;
  head->value = NULL;
  char *ptr = (char *)(json_string + 1);
  size_t count = 0;
  while (*ptr != '}') {
    if (*ptr == '\0' || *ptr != '"') {
      free_list_object(head);
      *error = JSON_EFMT;
      return NULL;
    }
    char *key = jsontok_parse_string((const char **)&ptr, error);
    if (!key) {
      free_list_object(head);
      *error = JSON_ENOMEM;
      return NULL;
    }
    if (*ptr != ':') {
      *error = JSON_EFMT;
      return NULL;
    }
    ptr ++;
    struct JsonToken *token = jsontok_parse_value((const char **)&ptr, error);
    if (!token) {
      free(key);
      free_list_object(head);
      return NULL;
    }
    struct JsonEntry *entry = malloc(sizeof(struct JsonEntry));
    if (!entry) {
      free(key);
      jsontok_free(token);
      free_list_object(head);
      *error = JSON_ENOMEM;
      return NULL;
    }
    entry->key = key;
    entry->value = token;
    struct Node *n = malloc(sizeof(struct Node));
    if (!n) {
      free(key);
      jsontok_free(token);
      free(entry);
      free_list_object(head);
      *error = JSON_ENOMEM;
      return NULL;
    }
    n->next = head;
    n->value = entry;
    head = n;
    if (*ptr == ',') ptr ++;
    count ++;
  }
  struct JsonObject *object = malloc(sizeof(struct JsonObject));
  if (!object) {
    free_list_object(head);
    *error = JSON_ENOMEM;
    return NULL;
  }
  struct JsonEntry **entries = malloc(count * sizeof(struct JsonEntry *));
  if (!entries) {
    free_list_object(head);
    *error = JSON_ENOMEM;
    return NULL;
  }
  size_t i = 0;
  while (head) {
    entries[i++] = (struct JsonEntry *)head->value;
    struct Node *t = head;
    head = head->next;
    free(t);
  }
  object->count = count;
  object->entries = entries;
  return object;
}

static struct JsonArray *jsontok_parse_array(const char *json_string, enum JsonError *error) {
  struct Node *head = malloc(sizeof(struct Node));
  if (!head) {
    *error = JSON_ENOMEM;
    return NULL;
  }
  head->next = NULL;
  head->value = NULL;
  char *ptr = (char *)(json_string + 1);
  size_t length = 0;
  while (*ptr != ']') {
    if (*ptr == '\0') {
      free_list_array(head);
      *error = JSON_EFMT;
      return NULL;
    }
    struct JsonToken *token = jsontok_parse_value((const char **)&ptr, error);
    if (!token) {
      free_list_array(head);
      return NULL;
    }
    struct Node *n = malloc(sizeof(struct Node));
    if (!n) {
      jsontok_free(token);
      free_list_array(head);
      *error = JSON_ENOMEM;
      return NULL;
    }
    n->next = head;
    n->value = token;
    head = n;
    if (*ptr == ',') ptr ++;
    length ++;
  }
  struct JsonToken **elements = malloc(length * sizeof(struct JsonToken *));
  if (!elements) {
    free_list_array(head);
    *error = JSON_ENOMEM;
    return NULL;
  }
  size_t i = length - 1;
  while (head) {
    struct JsonToken *token = head->value;
    elements[i--] = token;
    struct Node *t = head;
    head = head->next;
    free(t);
  }
  struct JsonArray *array = malloc(sizeof(struct JsonArray));
  if (!array) {
    size_t i = 0;
    for (; i < length; i++) jsontok_free(elements[i]);
    free(elements);
    *error = JSON_ENOMEM;
    return NULL;
  }
  array->length = length;
  array->elements = elements;
  return array;
}

struct JsonToken *jsontok_parse(const char *json_string, enum JsonError *error) {
  if (!json_string || strlen(json_string) == 0) {
    *error = JSON_EFMT;
    return NULL;
  }
  /* Numbers are either longs or doubles, so they have their own control path */
  char c = *json_string;
  if ((c < '0' && c > '9') || c == '-') {
    return jsontok_parse_number_token(&json_string, error);
  }
  struct JsonToken *token = malloc(sizeof(struct JsonToken));
  if (!token) {
    *error = JSON_ENOMEM;
    return NULL;
  }
  if (!memcmp(json_string, "true", 4)) {
    token->type = JSON_BOOLEAN;
    token->as_boolean = 1;
  } else if (!memcmp(json_string, "false", 5)) {
    token->type = JSON_BOOLEAN;
    token->as_boolean = 0;
  } else if (!memcmp(json_string, "null", 4)) {
    token->type = JSON_NULL;
  }
  switch (c) {
    case '"': {
      char *str = jsontok_parse_string(&json_string, error);
      if (!str) return NULL;
      token->type = JSON_STRING;
      token->as_string = str;
      break;
    }
    case '{': {
      struct JsonObject *object = jsontok_parse_object(json_string, error);
      if (!object) return NULL;
      token->type = JSON_OBJECT;
      token->as_object = object;
      break;
    }
    case '[': {
      struct JsonArray *array = jsontok_parse_array(json_string, error);
      if (!array) return NULL;
      token->type = JSON_ARRAY;
      token->as_array = array;
      break;
    }
    default:
      free(token);
      *error = JSON_EFMT;
      return NULL;
  }
  return token;
}

/* Similar to jsontok_parse but doesn't recursively expand objects and arrays. */
static struct JsonToken *jsontok_parse_value(const char **ptr, enum JsonError *error) {
  /* Numbers are either longs or doubles, so they have their own control path */
  char c = **ptr;
  if ((c >= '0' && c <= '9') || c == '-') {
    return jsontok_parse_number_token(ptr, error);
  }
  struct JsonToken *token = malloc(sizeof(struct JsonToken));
  if (!token) {
    *error = JSON_ENOMEM;
    return NULL;
  }
  if (!memcmp(*ptr, "true", 4)) {
    token->type = JSON_BOOLEAN;
    token->as_boolean = 1;
    *ptr += 4;
  } else if (!memcmp(*ptr, "false", 5)) {
    token->type = JSON_BOOLEAN;
    token->as_boolean = 0;
    *ptr += 5;
  } else if (!memcmp(*ptr, "null", 4)) {
    token->type = JSON_NULL;
    *ptr += 4;
  } else {
    switch (c) {
      case '"':
        token->type = JSON_STRING;
        token->as_string = jsontok_parse_string(ptr, error);
        break;
      case '{':
        token->type = JSON_WRAPPED_OBJECT;
        token->as_string = jsontok_wrap_object(ptr, error);
        break;
      case '[':
        token->type = JSON_WRAPPED_ARRAY;
        token->as_string = jsontok_wrap_array(ptr, error);
        break;
      default:
        free(token);
        *error = JSON_EFMT;
        return NULL;
    }
  }
  return token;
}

struct JsonToken *jsontok_unwrap(struct JsonToken *token, enum JsonError *error) {
  struct JsonToken *unwrapped_token = malloc(sizeof(struct JsonToken));
  if (!unwrapped_token) {
    *error = JSON_ENOMEM;
    return NULL;
  }
  if (token->type == JSON_WRAPPED_OBJECT) {
    struct JsonObject *object  = jsontok_parse_object(token->as_string, error);
    if (!object) return NULL;
    unwrapped_token->type = JSON_OBJECT;
    unwrapped_token->as_object = object;
    return unwrapped_token;
  }
  if (token->type == JSON_WRAPPED_ARRAY) {
    struct JsonArray *array = jsontok_parse_array(token->as_string, error);
    if (!array) return NULL;
    unwrapped_token->type = JSON_ARRAY;
    unwrapped_token->as_array = array;
  }
  free(unwrapped_token);
  *error = JSON_ETYPE;
  return NULL;
}
