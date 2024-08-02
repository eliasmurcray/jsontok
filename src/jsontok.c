#include "jsontok.h"
#include <stdio.h>

int main() {
  struct JsonToken token;
  token.type = JSON_STRING;
  token.as_string = "hello world";
  printf("%s\n", token.as_string);
  printf("%ld\n", token.as_long);
  return 0;
}
