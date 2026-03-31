This program combines multiple techniques to exercise all partition codes:

1. **Template Metaprogramming**: Specialized templates for each partition code 0-7 force the compiler to generate code for all valid cases at compile time.

2. **Volatile Control Flow**: The `execute_with_partition_code` function uses `volatile int code` to prevent constant folding and ensure all branches are compiled.

3. **Exhaustive Enumeration**: The main loop tests codes from -1 to 9, covering all valid codes (0-7) and illegal values.

4. **Multi-dimensional Arrays**: Uses 3D arrays with complex nesting patterns to trigger different broadcast scenarios.

5. **Mixed Partitioning Clauses**: Combines `gang`, `worker`, `vector`, and `redundant` clauses in various combinations.

6. **Prevention of Dead Code Elimination**: Final reductions and volatile writes ensure all code paths are retained.

**Compilation commands:**
