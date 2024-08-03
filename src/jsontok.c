#include "jsontok.h"

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
  unsigned char decimal = 0;
  while (1) {
    char c = *ptr;
    if (c > '0' && c < '9') continue;
    if (!decimal && c == '.') {
      decimal = 1;
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
    free(substr);
    if (errno || *endptr != '\0') {
      free(token);
      return NULL; /* failed to parse as double */
    }
    token->as_double = d;
  } else {
    long l = strtol(substr, &endptr, 10);
    free(substr);
    if (errno || *endptr != '\0') {
      free(token);
      return NULL; /* failed to parse as long */
    }
    token->as_long = l;
  }
  return token;
}

/* TODO Reduce reused between jsontok_parse_wrapped_object and jsontok_parse_wrapped_array */
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

static struct JsonToken *jsontok_parse_object(const char *json_string) {
  return (struct JsonToken *)json_string;
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
    case '.':
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

struct JsonToken *jsontok_unwrap(struct JsonToken *token) {
  if (token->type == JSON_WRAPPED_OBJECT) {
    return jsontok_parse_object(token->as_string);
  }
  if (token->type == JSON_WRAPPED_ARRAY) {
    return jsontok_parse_array(token->as_string);
  }
  return NULL; /* token type cannot be unwrapped */
}
