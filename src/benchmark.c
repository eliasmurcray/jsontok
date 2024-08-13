#include "jsontok.h"
#include <stdio.h>
#include <stdlib.h>
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

void benchmark(const char *path) {
  printf("Running %s benchmark...\n", path);
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
  long elapsed = ((end - start) * 1000000) / CLOCKS_PER_SEC;
  size_t bytes = strlen(json);
  double throughput = (double)bytes / (elapsed / 1e6) / (1024 * 1024);
  printf("Successfully parsed %s (%zu bytes) in %ldus (%.3f MB/s)\n\n", path, strlen(json), ((end - start) * 1000000) / CLOCKS_PER_SEC, throughput);

  free(json);
  jsontok_free(token);
}

int main() {
  benchmark("./samples/simple.json");
  benchmark("./samples/multidim_arr.json");
  benchmark("./samples/random.json");
  benchmark("./samples/rickandmorty.json");
  benchmark("./samples/food.json");
  benchmark("./samples/reddit.json");
}
