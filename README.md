# jsontok

jsontok is a lightweight library for efficiently parsing minified JSON in an iterative manner. It processes one layer at a time, returning nested objects and arrays as strings, which can be parsed further if needed. The library is designed to pass through the JSON string just once, optimizing performance.

I wish I could say this library was created to address gaps in other JSON libraries, but in reality, the only one I was referencing was [C-Simple-Json-Parser](https://github.com/forkachild/C-Simple-JSON-Parser). I initially used it in another project, but I encountered a few bugs that I couldn't resolve. Ultimately, I felt that a different interface might be more suitable, particularly from a JavaScript developer's perspective. My decision to minimize the use of macros in this code was also influenced by my experience reading the source of that library.

## Usage

Coming soon!

## Testing

The program is first tested against some minified JSON taken from [C-Simple-Json-Parser](https://github.com/forkachild/C-Simple-JSON-Parser) mainly to show performance. Then, unit tests are run against every case except for malloc failures, which are handled in the code but I still need to figure out how I want to test these failures. The code is also tested against Valgrind and/or MacOS leaks locally.
