# jsontok

jsontok is an incremental RFC 8259 compliant JSON parser written in ANSI C

## Features
- ANSI C compatible
- [RFC 8259](https://datatracker.ietf.org/doc/html/rfc8259) compliant

## Design
<a href="https://github.com/eliasmurcray/jsontok/blob/mainline/jsontok_dark.png" target="_blank">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="https://github.com/eliasmurcray/jsontok/blob/mainline/jsontok_dark.png">
    <source media="(prefers-color-scheme: light)" srcset="https://github.com/eliasmurcray/jsontok/blob/mainline/jsontok_light.png">
    <img height="500" alt="jsontok design diagram" src="https://github.com/eliasmurcray/jsontok/blob/mainline/jsontok_light.png">
  </picture>
</a>

<sub><sup>designed with <a href="https://draw.io/">draw.io</a></sup></sub>

jsontok parses JSON one layer at a time, treating nested objects and arrays as `JSON_SUB_OBJECT` and `JSON_SUB_ARRAY` tokens. These tokens are stringified subtrees that can be passed back into `jsontok_parse` to access deeper layers. This approach intends to conserve resources by avoiding unnecessary parsing of subtrees.

## Usage

You can either copy `include/jsontok.h` and `src/jsontok.c` into your program or you can clone this repo and run `make lib` to generate the static library.

## Documentation

### Errors

All errors are handled by a single enum type `JsonError`.

Here is the definition for all errors:

```c
enum JsonError {
  JSON_ENOERR,
  JSON_EFMT,
  JSON_ENOMEM,
  JSON_ETYPE,
};
```

There is a helper function which converts the errors to a readable string format:

```c
const char *jsontok_strerror(enum JsonError error) {
  switch (error) {
    case JSON_ENOERR:
      return "No error";
    case JSON_EFMT:
      return "Invalid format";
    case JSON_ETYPE:
      return "Invalid type";
    case JSON_ENOMEM:
      return "Out of memory";
    default:
      return "Unknown error";
  }
}
```

### Example

Here is an example following the one depicted in the design diagram:

```c
#include "jsontok.h"
#include <stdio.h>

int main () {
  const char *json = "{\n  \"num\": 42,  \"nested\": {\n  \"str\":\"foo\"\n  }\n}";
  enum JsonError error;
  struct JsonToken *token = jsontok_parse(json, &error);
  if (!token) {
    printf("Error parsing JSON: %s\n", jsontok_strerror(error));
    return NULL;
  }

  if (token->type == JSON_OBJECT) {
    struct JsonToken *num = jsontok_get(token->as_object, "num");
    if (num && num->type == JSON_NUMBER) {
      printf("num: %f\n", num->as_number);
    } else {
      printf("key 'num' not found\n");
    }

    struct JsonToken *nested = jsontok_get(token->as_object, "nested");
    if (nested && nested->type == JSON_SUB_OBJECT) {
      struct JsonToken *nested_obj = jsontok_parse(nested->as_string, &error);
      if (!nested_obj) {
        printf("Error parsing 'nested' JSON: %s\n", jsontok_strerror(error));
      } else {
      }
    } else {
      printf("key 'nested' not found\n");
    }
  }

  jsontok_free(token);
}
```

## Testing

Runs unit tests against every case and tested locally against Valgrind and/or MacOS leaks. You can run tests yourself by cloning the repo and running `make test`. You can add more tests in the `src/test.c` file.

## Benchmarks

Parses at about 290MB/s when parsing a 100,000 byte file. You can see benchmarks by cloning the repo and running `make benchmark`. You can add more benchmarks by adding to the `src/benchmark.c` file and adding more samples.
