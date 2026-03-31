This line is actually **invalid** because:
- `ExplicitClass(42)` creates a temporary via direct-initialization (allowed)
- `e2 = ExplicitClass(42)` attempts copy-initialization from the temporary
- The copy constructor is `explicit`, so this copy-initialization is not allowed

## Valid Alternatives

Here are valid ways to write this:
