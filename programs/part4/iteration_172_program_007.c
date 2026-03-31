Input: "foo(bar(baz))qux"
Processing:
- 'f','o','o' → handled by default case (advance)
- '(' → consume_balanced('(', ')') consumes "bar(baz)"
- 'q','u','x' → handled by default case
