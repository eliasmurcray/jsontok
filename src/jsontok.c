#include "jsontok.h"

static struct JsonToken *jsontok_parse_no_recursion(const char *json_string);

void jsontok_free(struct JsonToken *token) {
  switch(token->type) {
    case JSON_ARRAY: {
      unsigned int i;
      for (i = 0; i < token->as_array->length; i++) {
        jsontok_free(token->as_array->tokens[i]);
      }
      break;
    }
    case JSON_OBJECT: {
      unsigned int i;
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

static char escaped(char c) {
  switch (c) {
    case 'b': return '\b';
    case 'f': return '\f';
    case 'n': return '\n';
    case 'r': return '\r';
    case 't': return '\t';
    case '"': return '"';
    case '\\': return '\\';
    default: return '\0';
  }
}

struct JsonToken *jsontok_get(struct JsonObject *object, const char *key) {
  if (key == NULL) {
    return NULL; /* invalid key */
  }
  size_t length = strlen(key);
  if (length == 0) {
    return NULL; /* invalid key */
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
  return NULL; /* key not found */
}

static struct JsonToken *jsontok_parse_string(const char *json_string) {
  char *ptr = (char *)(json_string + 1);
  size_t length = 1;
  while (*ptr != '"') {
    if (*ptr == '\0') return NULL; /* failed to parse as string */
    ptr += *ptr == '\\' + 1;
    length ++;
  }
  char *substr = malloc(length);
  if (!substr) return NULL; /* malloc fail */
  size_t i = 0;
  char *nptr = (char *)(json_string + 1);
  for (; nptr != ptr; nptr++, i++) {
    if (*nptr != '\\') {
      substr[i] = *nptr;
      continue;
    }
    nptr ++;
    substr[i] = escaped(*nptr);
    if (substr[i] == '\0') {
      free(substr);
      return NULL; /* failed to parse as string invalid escape code */
    }
  }
  substr[i] = '\0';
  struct JsonToken *token = malloc(sizeof(struct JsonToken));
  if (!token) {
    free(substr);
    return NULL; /* malloc fail */
  }
  token->type = JSON_STRING;
  token->as_string = substr;
  return token;
}

static struct JsonToken *jsontok_parse_number(const char *json_string) {
  char *ptr = (char *)(json_string + 1);
  unsigned char needs_digit = *json_string == '-';
  unsigned char decimal = 1;
  while (1) {
    char c = *ptr++;
    if (c >= '0' && c <= '9') continue;
    if (needs_digit) return NULL; /* invalid format */
    if (decimal && c == '.') {
      decimal = 0;
      continue;
    }
    break;
  }
  size_t length = ptr - json_string + 1;
  char *substr = malloc(length);
  strncpy(substr, json_string, length - 1);
  substr[length] = '\0';
  struct JsonToken *token = malloc(sizeof(struct JsonToken));
  if (!token) {
    free(substr);
    return NULL; /* malloc fail */
  }
  token->type = JSON_NUMBER;
  errno = 0;
  char *endptr = NULL;
  if (decimal) {
    double d = strtod(substr, &endptr);
    if (errno || *endptr != '\0') {
      free(substr);
      free(token);
      return NULL; /* failed to parse as double */
    }
    free(substr);
    token->as_double = d;
  } else {
    long l = strtol(substr, &endptr, 10);
    if (errno || *endptr != '\0') {
      free(substr);
      free(token);
      return NULL; /* failed to parse as long */
    }
    free(substr);
    token->as_long = l;
  }
  return token;
}

/* TODO Reduce reused code between jsontok_parse_wrapped_object and jsontok_parse_wrapped_array */
static struct JsonToken *jsontok_parse_wrapped_object(const char* json_string) {
  char *ptr = (char *)(json_string + 1);
  size_t counter = 1;
  while (counter > 1 || *ptr != '}') {
    if (*ptr == '\0') return NULL; /* failed to parse as object */
    if (*ptr == '{') counter ++;
    else if (*ptr == '}') counter --;
    ptr += *ptr == '\\' + 1;
  }
  size_t length = ptr - json_string + 2;
  char *substr = malloc(length);
  if (!substr) return NULL; /* malloc fail */
  strncpy(substr, json_string, length - 1);
  substr[length] = '\0';
  struct JsonToken *token = malloc(sizeof(struct JsonToken));
  if (!token) {
    free(substr);
    return NULL; /* malloc fail */
  }
  token->type = JSON_WRAPPED_OBJECT;
  token->as_string = substr;
  return token;
}

static struct JsonToken *jsontok_parse_wrapped_array(const char* json_string) {
  char *ptr = (char *)(json_string + 1);
  size_t counter = 1;
  while (counter > 1 || *ptr != ']') {
    if (*ptr == '\0') return NULL; /* failed to parse as array */
    if (*ptr == '[') counter ++;
    else if (*ptr == ']') counter --;
    ptr += *ptr == '\\' + 1;
  }
  size_t length = ptr - json_string + 2;
  char *substr = malloc(length);
  if (!substr) return NULL; /* malloc fail */
  strncpy(substr, json_string, length - 1);
  substr[length] = '\0';
  struct JsonToken *token = malloc(sizeof(struct JsonToken));
  if (!token) {
    free(substr);
    return NULL; /* malloc fail */
  }
  token->type = JSON_WRAPPED_ARRAY;
  token->as_string = substr;
  return token;
}

static unsigned char skip_value(const char* ptr) {
  switch (*ptr) {
    case '"':
      ptr ++;
      while (*ptr != '"') {
        if (*ptr == '\0') return 1; /* invalid format */
        ptr += *ptr == '\\' + 1;
      }
      return 0;
    case '0':
    case '1':
    case '2':
    case '3': 
    case '4':
    case '5':
    case '6':
    case '7':
    case '8': 
    case '9': 
    case '-': {
      unsigned char needs_digit = *ptr == '-';
      unsigned char decimal = 1;
      ptr ++;
      while (1) {
        char c = *ptr++;
        if (c >= '0' && c <= '9') {
          needs_digit = 0;
          continue;
        }
        if (needs_digit) return 1; /* invalid format */
        if (decimal && c == '.') {
          decimal = 0;
          continue;
        }
        return !(c == ',' || c == '}');
      }
      break;
    }
    case '{': {
      ptr ++;
      size_t counter = 1;
      while (counter > 1 || *ptr != '}') {
        if (*ptr == '\0') return 1; /* invalid format */
        if (*ptr == '{') counter ++;
        else if (*ptr == '}') counter --;
        ptr += *ptr == '\\' + 1;
      } 
      return 0;
    }
    case '[': {
      ptr ++;
      size_t counter = 1;
      while (counter > 1 || *ptr != ']') {
        if (*ptr == '\0') return 1; /* invalid format */
        if (*ptr == '[') counter ++;
        else if (*ptr == ']') counter --;
        ptr += *ptr == '\\' + 1;
      } 
      return 0;
    }
    case 't':
      if (strncmp(ptr, "true", 4)) return 1; /* invalid format */
      ptr += 4;
      return 0;
    case 'f':
      if (strncmp(ptr, "false", 5)) return 1; /* invalid format */
      ptr += 5;
      return 0;
    case 'n':
      if (strncmp(ptr, "null", 4)) return 1; /* invalid format */
      ptr += 4;
      return 0;
  }
  return 1; /* invalid format */
}

static struct JsonToken *jsontok_parse_object(const char *json_string) {
  char *ptr = (char *)(json_string + 1);
  size_t count = 0;
  while (*ptr != '}') {
    if (*ptr != '"') return NULL; /* invalid format */
    ptr ++;
    while (*ptr != '"') {
      if (*ptr == '\0') return NULL; /* invalid format */
      ptr += *ptr == '\\' + 1;
    }
    ptr ++;
    if (*ptr != ':') return NULL; /* invalid format */
    ptr ++;
    if (skip_value(ptr)) return NULL; /* invalid format */
    count ++;
    ptr ++;
  }
  struct JsonEntry **entries = malloc(count * sizeof(struct JsonEntry *));
  if (!entries) return NULL; /* malloc fail */
  /* fill object values with another traversal of object */
  size_t j = 0;
  while (*ptr != '}') {
    if (*ptr != '"') return NULL; /* invalid format */
    ptr ++;
    char *nptr = ptr;
    size_t length = 1;
    while (*ptr != '"') {
      if (*ptr == '\0') return NULL; /* invalid format */
      ptr += *ptr == '\\' + 1;
      length ++;
    }
    char *key = malloc(length);
    if (!key) return NULL; /* malloc error */
    size_t i = 0;
    for (; nptr != ptr; nptr ++, i++) {
      if (*nptr != '\\') {
        key[i] = *nptr;
        continue;
      } 
      nptr ++;
      key[i] = escaped(*nptr);
      if (key[i] == '\0') {
        free(key);
        return NULL; /* failed to parse as string invalid escape code */
      }
    }
    key[i] = '\0';

    ptr ++;
    if (*ptr != ':') return NULL; /* invalid format */
    ptr ++;
    /* parse value */
    struct JsonToken* token = jsontok_parse_no_recursion(ptr);
    if (!token) {
      free(key);
      free(entries);
      return NULL; /* ??? */
    }
    skip_value(ptr);

    struct JsonEntry *entry = malloc(sizeof(struct JsonEntry));
    if (!entry) {
      free(key);
      free(entries);
      free(entry);
      return NULL;
    }
    entry->key = key;
    entry->value = token;
    entries[j] = entry;

    j ++;
    ptr ++;
  }
  struct JsonObject *object = malloc(sizeof(struct JsonObject));
  if (!object) {
    free(entries);
    return NULL; /* malloc fail */
  }
  object->count = count;
  object->entries = entries;
  struct JsonToken *token = malloc(sizeof(struct JsonToken));
  if (!token) {
    free(entries);
    free(object);
    return NULL; /* malloc fail */
  }
  token->type = JSON_OBJECT;
  token->as_object = object;
  return NULL;
}

static struct JsonToken *jsontok_parse_array(const char *json_string) {
  return (struct JsonToken *)json_string;
}

static struct JsonToken *jsontok_parse_boolean(const char *json_string) {
  if (strlen(json_string) == 4 && json_string[0] == 't' && json_string[1] == 'r'
      && json_string[2] == 'u' && json_string[3] == 'e') {
    struct JsonToken *token = malloc(sizeof(struct JsonToken));
    if (!token) return NULL; /* malloc fail */
    token->type = JSON_BOOLEAN;
    token->as_boolean = 1;
    return token;
  }
  if (strlen(json_string) == 5 && json_string[0] == 'f' && json_string[1] == 'a'
      && json_string[2] == 'l' && json_string[3] == 's' && json_string[4] == 'e') {
    struct JsonToken *token = malloc(sizeof(struct JsonToken));
    if (!token) return NULL; /* malloc fail */
    token->type = JSON_BOOLEAN;
    token->as_boolean = 0;
  }
  return NULL; /* attempted to parse as boolean but failed */
}

static struct JsonToken *jsontok_parse_null(const char *json_string) {
  if (strlen(json_string) == 4 && json_string[1] == 'u'
      && json_string[2] == 'l' && json_string[3] == 'l') {
    struct JsonToken *token = malloc(sizeof(struct JsonToken));
    if (!token) return NULL; /* malloc fail */
    token->type = JSON_NULL;
    return token;
  }
  return NULL; /* attempted to parse as null but failed */
}

struct JsonToken *jsontok_parse(const char *json_string) {
  if (json_string == NULL) return NULL; /* parse error invalid input */
  switch (*json_string) {
    case '"':
      return jsontok_parse_string(json_string);
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '-':
      return jsontok_parse_number(json_string);
      break;
    case '{':
      return jsontok_parse_object(json_string);
      break;
    case '[':
      return jsontok_parse_array(json_string);
      break;
    case 't':
    case 'f':
      return jsontok_parse_boolean(json_string);
      break;
    case 'n':
      return jsontok_parse_null(json_string);
      break;
  }
  return NULL; /* parse error unknown type */
}

static struct JsonToken *jsontok_parse_no_recursion(const char *json_string) {
  if (json_string == NULL) return NULL; /* parse error invalid input */
  switch (*json_string) {
    case '"':
      return jsontok_parse_string(json_string);
      break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    case '-':
      return jsontok_parse_number(json_string);
      break;
    case '{':
      return jsontok_parse_wrapped_object(json_string);
      break;
    case '[':
      return jsontok_parse_wrapped_array(json_string);
      break;
    case 't':
    case 'f':
      return jsontok_parse_boolean(json_string);
      break;
    case 'n':
      return jsontok_parse_null(json_string);
      break;
  }
  return NULL; /* parse error unknown type */
}

struct JsonToken *jsontok_unwrap(struct JsonToken *token) {
  if (token->type == JSON_WRAPPED_OBJECT) {
    return jsontok_parse_object(token->as_string);
  }
  if (token->type == JSON_WRAPPED_ARRAY) {
    return jsontok_parse_array(token->as_string);
  }
  return NULL; /* token type cannot be unwrapped */
}
