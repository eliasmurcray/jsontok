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

Documentation coming soon! For now, take a look at `include/jsontok.h` for an overview of available functions.

## Testing

Runs unit tests against every case and tested locally against Valgrind and/or MacOS leaks. You can run tests yourself by cloning the repo and running `make test`. You can add more tests in the `src/test.c` file.

## Benchmarks

Parses at about 290MB/s when parsing a 100,000 byte file. You can see benchmarks by cloning the repo and running `make benchmark`. You can add more benchmarks by adding to the `src/benchmark.c` file and adding more samples.
