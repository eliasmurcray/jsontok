#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jsontok.h"

void test_parse_valid_json() {
  enum JsonError error = JSON_ENOERR;
  const char *json_string = "{\"key\":\"value\",\"number\":42,\"array\":[1,2,3],\"nested\":{\"inner_key\":\"inner_value\"}}";
  struct JsonToken *token = jsontok_parse(json_string, &error);

  assert(token != NULL);
  assert(error == JSON_ENOERR);
  assert(token->type == JSON_OBJECT);

  struct JsonToken *value_token = jsontok_get(token->as_object, "key");
  assert(value_token != NULL);
  assert(value_token->type == JSON_STRING);
  assert(strcmp(value_token->as_string, "value") == 0);

  struct JsonToken *number_token = jsontok_get(token->as_object, "number");
  assert(number_token != NULL);
  assert(number_token->type == JSON_NUMBER);
  assert(number_token->as_number == 42);

  struct JsonToken *array_token = jsontok_get(token->as_object, "array");
  assert(array_token != NULL);
  assert(array_token->type == JSON_WRAPPED_ARRAY);

  struct JsonToken *nested_token = jsontok_get(token->as_object, "nested");
  assert(nested_token != NULL);
  assert(nested_token->type == JSON_WRAPPED_OBJECT);
  struct JsonToken *unwrapped_token = jsontok_parse(nested_token->as_string, &error);
  assert(unwrapped_token != NULL);
  assert(unwrapped_token->type == JSON_OBJECT);

  struct JsonToken *inner_key_token = jsontok_get(unwrapped_token->as_object, "inner_key");
  assert(inner_key_token != NULL);
  assert(inner_key_token->type == JSON_STRING);
  assert(strcmp(inner_key_token->as_string, "inner_value") == 0);

  jsontok_free(token);
  jsontok_free(unwrapped_token);
}

void test_parse_invalid_json() {
  enum JsonError error = JSON_ENOERR;
  const char *json_string = "{\"key\":\"value\",\"number\":42,\"array\":[1,2,3,],\"nested\":{\"inner_key\":\"inner_value\"}";
  struct JsonToken *token = jsontok_parse(json_string, &error);

  assert(token == NULL);
  assert(error == JSON_EFMT);
}

void test_unwrap_json_wrapped_object() {
  enum JsonError error = JSON_ENOERR;
  const char *json_string = "{\"wrapped\":{\"key\":\"value\"}}";
  struct JsonToken *token = jsontok_parse(json_string, &error);

  assert(token != NULL);
  assert(error == JSON_ENOERR);

  struct JsonToken *wrapped_token = jsontok_get(token->as_object, "wrapped");
  assert(wrapped_token != NULL);
  assert(wrapped_token->type == JSON_WRAPPED_OBJECT);

  struct JsonToken *unwrapped_token = jsontok_parse(wrapped_token->as_string, &error);
  assert(unwrapped_token != NULL);
  assert(unwrapped_token->type == JSON_OBJECT);

  struct JsonToken *value_token = jsontok_get(unwrapped_token->as_object, "key");
  assert(value_token != NULL);
  assert(value_token->type == JSON_STRING);
  assert(strcmp(value_token->as_string, "value") == 0);

  jsontok_free(token);
  jsontok_free(unwrapped_token);
}

void test_get_nonexistent_key() {
  enum JsonError error = JSON_ENOERR;
  const char *json_string = "{\"key\":\"value\"}";
  struct JsonToken *token = jsontok_parse(json_string, &error);

  assert(token != NULL);
  assert(error == JSON_ENOERR);

  struct JsonToken *missing_token = jsontok_get(token->as_object, "nonexistent");
  assert(missing_token == NULL);

  jsontok_free(token);
}

int main() {
  printf("Running test_parse_valid_json...");
  test_parse_valid_json();
  printf(" PASSED\n");
  printf("Running test_parse_invalid_json...");
  test_parse_invalid_json();
  printf(" PASSED\n");
  printf("Running test_unwrap_json_wrapped_object...");
  test_unwrap_json_wrapped_object();
  printf(" PASSED\n");
  printf("Running test_get_nonexistent_key...");
  test_get_nonexistent_key();
  printf(" PASSED\n");

  return 0;
}
