# jsontok

jsontok is a small library for parsing minfied JSON iteratively. Instead of parsing an entire tree recursively, it parses one layer at a time to save resources which would have been used to parse subtrees and returns nested objects and arrays as strings which can then be parsed again if you'd like to access that branch. There are a few other design decisions which allow the the parser to only iterate over the string once.

## Usage

Coming soon!

## Testing

The program is first tested against some minified JSON taken from [C-Simple-Json-Parser](https://github.com/forkachild/C-Simple-JSON-Parser) mainly to show performance. Then, unit tests are run against every case except for malloc failures, which are handled in the code but I still need to figure out how I want to test these failures. Feel free to add more tests.
