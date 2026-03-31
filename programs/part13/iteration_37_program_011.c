This program systematically exercises all partition codes through:

1. **Exhaustive Enumeration**: Iterates over codes -1 through 100, covering all valid (0-7) and illegal values
2. **Multi-dimensional Arrays**: Uses 3D arrays with complex nested loops
3. **Explicit Partitioning**: Applies `gang`, `worker`, `vector` clauses individually and in combination
4. **Template Metaprogramming**: Compile-time instantiation of all partition patterns
5. **Volatile Variables**: Prevents constant folding and ensures all code paths are generated
6. **Mixed Clauses**: Combines `redundant`, `static:*`, `num:`, `length:` clauses to stress the compiler
7. **Both `parallel` and `kernels`**: Uses both OpenACC constructs for broader coverage

**Compilation commands:**
