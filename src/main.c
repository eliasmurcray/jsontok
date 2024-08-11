#include "jsontok.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

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
  fread(buffer, 1, len, file);
  buffer[len] = '\0';
  return buffer;
}

void test_parse(const char *path) {
  printf("Running %s parse test...\n", path);
  char *json = read_file(path);
  if (json == NULL) {
    fprintf(stderr, "Failed to get %s\n", path);
    return;
  }
  struct timespec end, start;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  enum JsonError error;
  struct JsonToken *token = jsontok_parse(json, &error);
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  if (token == NULL) {  
    free(json);
    fprintf(stderr, "Failed to parse JSON: %s\n", jsontok_strerror(error));
    return;
  }
  uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  printf("Successfully parsed %s (%zu bytes) in %" PRIu64 "us\n\n", path, strlen(json), delta_us);
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
  if (token == NULL) {
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
}
