# jsontok

jsontok is a lightweight library for efficiently parsing minified JSON in an iterative manner. It processes one layer at a time, returning nested objects and arrays as strings, which can be parsed further if needed. The library is designed to pass through the JSON string just once, optimizing performance.

## Usage

Coming soon!

## Testing

The program is first tested against some minified JSON taken from [C-Simple-Json-Parser](https://github.com/forkachild/C-Simple-JSON-Parser) mainly to show performance. Then, unit tests are run against every case except for malloc failures, which are handled in the code but I still need to figure out how I want to test these failures. Feel free to add more tests.
