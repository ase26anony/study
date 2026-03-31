This program combines multiple techniques to exercise the partition code mapping:

1. **Template Metaprogramming**: Specialized templates for partition codes 0-7 generate distinct OpenACC constructs at compile time.

2. **Volatile Control Flow**: The `selector` variable prevents constant folding, ensuring all switch cases remain in the compiled code.

3. **Exhaustive Enumeration**: Tests partition codes from -1 to 9, covering all valid cases (0-7) and illegal values.

4. **Multi-dimensional Arrays**: Uses 3D arrays with complex broadcasting patterns across different partitioning schemes.

5. **Mixed Partitioning**: Combines `gang`, `worker`, `vector`, and `redundant` clauses in various combinations.

6. **Prevention of Dead Code Elimination**: Final reductions and output ensure all code paths contribute to observable results.

**Compilation commands:**
