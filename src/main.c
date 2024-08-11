#include "jsontok.h"
#include <stdio.h>

int main() {
  const char* my_json = "{\"foo\":\"bar\",\"baz\":1,\"nest\":{\"pi\":3.41}}";
  printf("Input: %s\n", my_json);
  enum JsonError error;
  struct JsonToken *token = jsontok_parse(my_json, &error);
  if (!token) {
    fprintf(stderr, "Error parsing JSON: %s\n", jsontok_strerror(error));
    return 1;
  }
  struct JsonToken *foo = jsontok_get(token->as_object, "foo");
  if (foo && foo->type == JSON_STRING) {
    printf("value of foo is %s\n", foo->as_string);
  }
  struct JsonToken *bar = jsontok_get(token->as_object, "baz");
  if (bar && bar->type == JSON_LONG) {
    printf("value of bar is %ld\n", bar->as_long);
  }
  struct JsonToken *nest = jsontok_get(token->as_object, "nest");
  if (nest && nest->type == JSON_WRAPPED_OBJECT) {
    struct JsonToken *obj = jsontok_unwrap(nest, &error);
    if (obj && obj->type == JSON_OBJECT) {
      struct JsonToken *pi = jsontok_get(obj->as_object, "pi");
      if (pi && pi->type == JSON_DOUBLE) {
        printf("value of nest.pi is %f\n", pi->as_double);
      }
    }
    jsontok_free(obj);
  }
  jsontok_free(token);
}
