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

### Types

All JSON is parsed into struct `JsonToken` and has the following types:
```c
enum JsonType {
  JSON_STRING,
  JSON_NUMBER,
  JSON_OBJECT,
  JSON_ARRAY,
  JSON_BOOLEAN,
  JSON_NULL,
  JSON_SUB_OBJECT,
  JSON_SUB_ARRAY,
};
```

The tokens all have a anonymous union attached to them that allow you to get data from it.

```
// struct JsonToken *token;

token->type // (enum JsonType)

// JSON_STRING
token->as_string // (char *)
// JSON_NUMBER
token->as_number // (double)
// JSON_OBJECT
token->as_object // (struct JsonObject *)
// JSON_ARRAY
token->as_array // (struct JsonArray *)
// JSON_BOOLEAN
token->as_boolean // (unsigned char)
// JSON_NULL
// This case should be handled yourself
// JSON_SUB_OBJECT
token->as_string (char *)
// JSON_SUB_ARRAY
token->as_string (char *)
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
    return 1;
  }
  if (token->type != JSON_OBJECT) {
    printf("json is not of type object\n");
    jsontok_free(token);
    return 1;
  }

  struct JsonToken *num = jsontok_get(token->as_object, "num");
  if (!num) {
    printf("key 'num' not found\n");
    jsontok_free(token);
    return 1;
  }
  if (num->type != JSON_NUMBER) {
    printf("key 'num' is not of type number\n");
    jsontok_free(token);
    return 1;
  }
  printf("num: %f\n", num->as_number);

  struct JsonToken *nested = jsontok_get(token->as_object, "nested");
  if (!nested) {
    printf("key 'nested' not found\n");
    jsontok_free(token);
    return 1;
  }
  if (nested->type != JSON_SUB_OBJECT) {
    printf("key 'nested' is not a nested object\n");
    jsontok_free(token);
    return 1;
  }

  struct JsonToken *nested_obj = jsontok_parse(nested->as_string, &error);
  if (!nested_obj) {
    printf("Error parsing 'nested' JSON: %s\n", jsontok_strerror(error));
    jsontok_free(token);
    return 1;
  }
  if (nested_obj->type != JSON_OBJECT) {
    printf("nested_obj is not of type object\n");
    jsontok_free(token);
    jsontok_free(nested_obj);
    return 1;
  }
  struct JsonToken *str = jsontok_get(nested_obj->as_object, "str");
  if (!str) {
    printf("key 'nested.str' not found\n");
    jsontok_free(token);
    jsontok_free(nested_obj);
    return 1;
  }
  if (str->type != JSON_STRING) {
    printf("key 'nested.str' is not of type string\n");
    jsontok_free(token);
    jsontok_free(nested_obj);
    return 1;
  }
  printf("nested.str: %s\n", str->as_string);
  jsontok_free(nested_obj);
  jsontok_free(token);
}
```

Output:
```
num: 42.000000
nested.str: foo
```

You try this out yourself by cloning this repo and running `make example`.

## Testing

Runs unit tests against every case and tested locally against Valgrind and/or MacOS leaks. You can run tests yourself by cloning the repo and running `make test`. You can add more tests in the `src/test.c` file.

## Benchmarks

Parses at about 290MB/s when parsing a 100,000 byte file. You can see benchmarks by cloning the repo and running `make benchmark`. You can add more benchmarks by adding to the `src/benchmark.c` file and adding more samples.
