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

struct JsonToken {
  enum JsonType type;
  union {
    const struct JsonObject* as_object;
    const struct JsonArray* as_array;
    const char *as_string;
    long as_long;
    double as_double;
  };
};

void jsontok_free(struct JsonToken* token);

struct JsonToken* jsontok_parse(const char* str);

#endif
