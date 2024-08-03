#include "jsontok.h"
#include <stdio.h>

int main() {
  const char* my_json = "1234";
  struct JsonToken *token = jsontok_parse(my_json);
  if (!token) {
    printf("token is null\n");
    return 1;
  }
  if (token->type == JSON_NUMBER) {
    printf("%ld\n", token->as_long);
  }
  jsontok_free(token);
}
