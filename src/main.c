#include "jsontok.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char *read_file(const char *path) {
  FILE *file = fopen(path, "r");
  if (!file) return NULL;
  fseek(file, 0, SEEK_END);
  long len = ftell(file);
  fseek(file, 0, SEEK_SET);
  char *buffer = malloc(len + 1);
  if (!buffer) {
    fclose(file);
    return NULL;
  }
  if (!fread(buffer, 1, len, file)) {
    fclose(file);
    free(buffer);
    return NULL;
  }
  buffer[len] = '\0';
  fclose(file);
  return buffer;
}

void test_parse(const char *path) {
  printf("Running %s parse test...\n", path);
  char *json = read_file(path);
  if (json == NULL) {
    fprintf(stderr, "Failed to get %s\n", path);
    return;
  }
  clock_t start = clock();
  enum JsonError error;
  struct JsonToken *token = jsontok_parse(json, &error);
  clock_t end = clock();
  if (token == NULL) {  
    free(json);
    fprintf(stderr, "Failed to parse JSON: %s\n", jsontok_strerror(error));
    return;
  }
  printf("Successfully parsed %s (%zu bytes) in %ldus\n\n", path, strlen(json), ((end - start) * 1000000) / CLOCKS_PER_SEC);

  free(json);
  jsontok_free(token);
}

int main() {
  test_parse("./samples/simple.json");
  test_parse("./samples/multidim_arr.json");
  test_parse("./samples/random.json");
  test_parse("./samples/rickandmorty.json");
  test_parse("./samples/food.json");
  test_parse("./samples/reddit.json");
  
  printf("Parsing last value of food json\n");
  char *json = read_file("./samples/food.json");
  enum JsonError error;
  struct JsonToken *token = jsontok_parse(json, &error);
  if (!token) {
    free(json);
    fprintf(stderr, "failed to parse json %s\n", jsontok_strerror(error));
    return 1;
  }
  free(json);
  if (token->type == JSON_OBJECT) {
    struct JsonToken *status_verbose = jsontok_get(token->as_object, "status_verbose");
    if (status_verbose) {
      printf("status_verbose: %s\n", status_verbose->as_string);
    }
  }
  jsontok_free(token);

  json = read_file("./samples/simple.json");
  token = jsontok_parse(json, &error);
  if (!token) {
    free(json);
    fprintf(stderr, "failed to parse json %s\n", jsontok_strerror(error));
    return 1;
  }
  free(json);
  if (token->type == JSON_OBJECT) {
    struct JsonToken *data = jsontok_get(token->as_object, "data");
    if (data) {
      printf("data: %s\n", data->as_string);
    }
  }
  jsontok_free(token);
}
