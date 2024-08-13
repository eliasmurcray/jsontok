# jsontok

jsontok is a lightweight library for efficiently parsing JSON.

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

Coming soon!

## Testing

The program is first tested against some minified JSON taken from [C-Simple-JSON-Parser](https://github.com/forkachild/C-Simple-JSON-Parser) primarily for benchmarking. Then, unit tests are run against every case. The code is also tested against Valgrind and/or MacOS leaks locally.

## Benchmarks

About 8.6x faster than C-Simple-JSON-Parser, more coming soon!
