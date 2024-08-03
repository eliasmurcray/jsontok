#include "jsontok.h"
#include <stdio.h>

int main() {
  const char* my_json = "123.45";
  struct JsonToken *token = jsontok_parse(my_json);
  if (!token) {
    printf("token is null\n");
    return 1;
  }
  if (token->type == JSON_NUMBER) {
    printf("%f\n", token->as_double);
  }
  jsontok_free(token);
}
