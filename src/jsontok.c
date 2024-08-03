#include "jsontok.h"

void jsontok_free(struct JsonToken *token) {
  switch(token->type) {
    case JSON_ARRAY: {
      unsigned int i;
      for (i = 0; i < token->as_array->length; i++) {
        free(token->as_array->tokens[i]);
      }
      break;
    }
    case JSON_OBJECT: {
      unsigned int i;
      for (i = 0; i < token->as_object->count; i++) {
        free(token->as_object->entries[i]->key); /* may not be necessary (char*) */
        free(token->as_object->entries[i]->value);
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
  char *ptr = (char *)json_string;
  ptr ++;
  while (*ptr != '\0') {
    ptr += *ptr == '\\' ? 2 : 1;
    if (*ptr == '"') break;
  }
  if (ptr - json_string == 1) return NULL; /* tried to parse as string and failed */
  struct JsonToken *token = malloc(sizeof(struct JsonToken));
  if (!token) return NULL; /* malloc fail */
  token->type = JSON_STRING;
  if (ptr - json_string == 2) {
    token->as_string = "";
    return token;
  }
  size_t length = ptr - json_string - 2;
  char *substr = malloc(ptr - json_string - 2);
  if (!substr) return NULL; /* malloc fail */
  strncpy(substr, json_string + 1, length);
  token->as_string = substr;
  return token;
}

static struct JsonToken *jsontok_parse_number(const char *json_string) {
  json_string++;
}

static struct JsonToken *jsontok_parse_object(const char *json_string) {
  char *ptr = (char *)json_string;
  ptr ++;
  while (*ptr != '\0') {
    if (*ptr != '"') return NULL; /* expected '"' at position (ptr - json_string) */
    if (*ptr == '}') break;
  }
}

static struct JsonToken *jsontok_parse_array(const char *json_string) {
  
}

static struct JsonToken *jsontok_parse_boolean(const char *json_string) {
  if (strlen(json_string) == 4 && json_string[0] == 't' && json_string[1] == 'r'
      && json_string[2] == 'u' && json_string[3] == 'e') {
    struct JsonToken *token = malloc(sizeof(struct JsonToken));
    token->type = JSON_BOOLEAN;
    token->as_boolean = 1;
    return token;
  }
  if (strlen(json_string) == 5 && json_string[0] == 'f' && json_string[1] == 'a'
      && json_string[2] == 'l' && json_string[3] == 's' && json_string[4] == 'e') {
    struct JsonToken *token = malloc(sizeof(struct JsonToken));
    token->type = JSON_BOOLEAN;
    token->as_boolean = 0;
  }
  return NULL; /* attempted to parse as boolean but failed */
}

static struct JsonToken *jsontok_parse_null(const char *json_string) {
  if (strlen(json_string) == 4 && json_string[1] == 'u'
      && json_string[2] == 'l' && json_string[3] == 'l') {
    struct JsonToken *token = malloc(sizeof(struct JsonToken));
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
