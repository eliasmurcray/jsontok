#ifndef JSONTOK_H
#define JSONTOK_H

#include <stdlib.h>
#include <string.h>

/*
  TODO Error codes
  INVALID_TYPE
*/

enum JsonType {
  JSON_STRING,
  JSON_NUMBER,
  JSON_OBJECT,
  JSON_ARRAY,
  JSON_BOOLEAN,
  JSON_NULL,
  JSON_WRAPPED_OBJECT,
  JSON_WRAPPED_ARRAY,
};

struct JsonToken;

struct JsonArray {
  unsigned int length;
  struct JsonToken **tokens;
};

struct JsonEntry {
  char *key;
  struct JsonToken *value;
};

struct JsonObject {
  unsigned int count;
  const struct JsonEntry **entries;
};

struct JsonToken {
  enum JsonType type : 4;
  union {
    struct JsonObject *as_object;
    struct JsonArray *as_array;
    char *as_string;
    long as_long;
    double as_double;
    unsigned char as_boolean;
  };
};

/**
 * @brief Frees a JsonToken and its children, if any.
 *
 * @param token The JsonToken to be freed.
 */
void jsontok_free(struct JsonToken *token);

/**
 * @brief Parses a JSON string and returns a JsonToken.
 *
 * @param json_string The JSON string to parse.
 * @return A pointer to a JsonToken representing the parsed JSON, or NULL if an error occurs.
 *         In case of an error, errno is set.
 */
struct JsonToken *jsontok_parse(const char *json_string);

/**
 * @brief Retrieves the value for a specified key in a JSON object.
 *
 * @param object The JSON object to search.
 * @param key The key to find.
 * @return The value associated with the key, or NULL if not found or an error occurs. In case of an error, errno is set.
 */
struct JsonToken *jsontok_get(struct JsonObject *object, const char *key);

/**
 * @brief Unwraps a JSON wrapped token.
 *
 * This function unwraps a token of type JSON_WRAPPED_OBJECT or JSON_WRAPPED_ARRAY into a token
 * of type JSON_OBJECT or JSON_ARRAY if possible.
 *
 * @param token The JsonToken to unwrap.
 * @return The unwrapped JsonToken, or NULL if unwrapping is not possible.
 */
struct JsonToken* jsontok_unwrap(struct JsonToken* token);

#endif
