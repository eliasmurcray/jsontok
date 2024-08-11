#include "jsontok.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <inttypes.h>

const char *read_file(const char *path) {
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
  const char *json = read_file(path);
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
    fprintf(stderr, "Failed to parse JSON: %s\n", jsontok_strerror(error));
    return;
  }
  jsontok_free(token);

  uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  printf("Successfully parsed %s (%zu bytes) in %" PRIu64 "us\n\n", path, strlen(json), delta_us);
}

int main() {
  test_parse("./samples/simple.json");
  test_parse("./samples/food.json");
  test_parse("./samples/reddit.json");
}
