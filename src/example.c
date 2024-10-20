#include <stdio.h>

#include "jsontok.h"

int main() {
  const char *json = "{\n  \"num\": 42,  \"nested\": {\n  \"str\":\"foo\"\n  }\n}";
  enum JsonError error;
  struct JsonToken *token = jsontok_parse(json, &error);
  if (!token) {
    printf("Error parsing JSON: %s\n", jsontok_strerror(error));
    return 1;
  }
  if (token->type != JSON_OBJECT) {
    printf("json is not of type object\n");
    jsontok_free(token);
    return 1;
  }

  struct JsonToken *num = jsontok_get(token->as_object, "num");
  if (!num) {
    printf("key 'num' not found\n");
    jsontok_free(token);
    return 1;
  }
  if (num->type != JSON_NUMBER) {
    printf("key 'num' is not of type number\n");
    jsontok_free(token);
    return 1;
  }
  printf("num: %f\n", num->as_number);

  struct JsonToken *nested = jsontok_get(token->as_object, "nested");
  if (!nested) {
    printf("key 'nested' not found\n");
    jsontok_free(token);
    return 1;
  }
  if (nested->type != JSON_SUB_OBJECT) {
    printf("key 'nested' is not a nested object\n");
    jsontok_free(token);
    return 1;
  }

  struct JsonToken *nested_obj = jsontok_parse(nested->as_string, &error);
  if (!nested_obj) {
    printf("Error parsing 'nested' JSON: %s\n", jsontok_strerror(error));
    jsontok_free(token);
    return 1;
  }
  if (nested_obj->type != JSON_OBJECT) {
    printf("nested_obj is not of type object\n");
    jsontok_free(token);
    jsontok_free(nested_obj);
    return 1;
  }
  struct JsonToken *str = jsontok_get(nested_obj->as_object, "str");
  if (!str) {
    printf("key 'nested.str' not found\n");
    jsontok_free(token);
    jsontok_free(nested_obj);
    return 1;
  }
  if (str->type != JSON_STRING) {
    printf("key 'nested.str' is not of type string\n");
    jsontok_free(token);
    jsontok_free(nested_obj);
    return 1;
  }
  printf("nested.str: %s\n", str->as_string);
  jsontok_free(nested_obj);
  jsontok_free(token);
}
