# jsontok

jsontok is a lightweight library for efficiently parsing JSON.

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="https://github.com/eliasmurcray/jsontok/blob/mainline/jsontok_dark.png">
  <source media="(prefers-color-scheme: light)" srcset="https://github.com/eliasmurcray/jsontok/blob/mainline/jsontok_light.png">
  <img width="500" alt="jsontok design diagram" src="https://github.com/eliasmurcray/jsontok/blob/mainline/jsontok_light.png">
</picture>

## Features
- ANSI C compatible
- [RFC 8259](https://datatracker.ietf.org/doc/html/rfc8259) compatible

## Usage

Coming soon!

## Testing

The program is first tested against some minified JSON taken from [C-Simple-Json-Parser](https://github.com/forkachild/C-Simple-JSON-Parser) mainly to show performance. Then, unit tests are run against every case except for malloc failures, which are handled in the code but I still need to figure out how I want to test these failures. The code is also tested against Valgrind and/or MacOS leaks locally.

### Benchmarks

Coming soon!
