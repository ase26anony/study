This program systematically exercises all partition codes through:

1. **Template Specializations (0-7)**: Each partition code 0-7 has a dedicated template specialization with the corresponding OpenACC partitioning clauses.

2. **Runtime Testing with Volatile**: Uses `volatile int` selectors to force the compiler to generate code for all branches, including illegal values (-1, 8, 9, 100).

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array `md_data[8][8][8]` with complex nested loops to trigger broadcast operations.

4. **Exhaustive Enumeration**: Tests all values 0-7 plus illegal values both at compile-time (templates) and runtime (volatile).

5. **Mixed Partitioning Schemes**: Combines `gang`, `worker`, `vector`, and `redundant` clauses in various combinations.

6. **Prevention of Dead Code Elimination**: Includes reductions and final computations that use all results.

**Compilation commands:**
