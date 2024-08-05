#include "jsontok.h"
#include <stdio.h>

int main() {
  char *error = NULL;
  const char* my_json = "[123.45,\"foo\",null,0,[0,true,0.24]]";
  printf("input: '%s'\n", my_json);
  struct JsonToken *json = jsontok_parse(my_json, &error);
  if (!json) {
    printf("%s\n", error);
    return 1;
  }
  if (json->type == JSON_ARRAY) {
    struct JsonArray *array = json->as_array;
    size_t i = 0;
    for (; i < array->length; i++) {
      switch (array->elements[i]->type) {
        case JSON_LONG:
          printf("%ld\n", array->elements[i]->as_long);
          break;
        case JSON_DOUBLE:
          printf("%f\n", array->elements[i]->as_double);
          break;
        case JSON_WRAPPED_OBJECT:
        case JSON_WRAPPED_ARRAY:
        case JSON_STRING:
          printf("%s\n", array->elements[i]->as_string);
          break;
        case JSON_BOOLEAN:
          printf("%c\n", array->elements[i]->as_boolean);
          break;
        case JSON_NULL:
          printf("null\n");
          break;
        default:
          break;
      }
    }
  }
  free(error);
  jsontok_free(json);
}
