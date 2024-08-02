#ifndef JSONTOK_H
#define JSONTOK_H

enum JsonType {
  JSON_STRING,
  JSON_NUMBER,
  JSON_OBJECT,
  JSON_ARRAY,
  JSON_BOOLEAN,
  JSON_NULL,
};

struct JsonArray {
  unsigned int length;
  struct JsonToken* elements;
};

struct JsonObject {
  unsigned int length;
  struct JsonToken* tokens;
};

struct JsonToken {
  enum JsonType type : 3;
  union {
    const struct JsonObject* as_object;
    const struct JsonArray* as_array;
    const char *as_string;
    long as_long;
    double as_double;
  };
};

/**
 * @brief Frees a JsonToken and its children, if any.
 *
 * @param token The JsonToken to be freed.
 */
void jsontok_free(struct JsonToken* token);

/**
 * @brief Parses a JSON string and returns a JsonToken.
 *
 * @param json_string The JSON string to parse.
 * @return A pointer to a JsonToken representing the parsed JSON, or NULL if an error occurs.
 *         In case of an error, errno is set.
 */
struct JsonToken* jsontok_parse(const char* json_string);

/**
 * @brief Retrieves the value for a specified key in a JSON object.
 *
 * @param object The JSON object to search.
 * @param key The key to find.
 * @return The value associated with the key, or NULL if not found or an error occurs. In case of an error, errno is set.
 */
struct JsonToken* jsontok_get(struct JsonObject* object, const char* key);

#endif
