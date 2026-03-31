This program combines multiple techniques to exercise the uncovered partition code mapping:

1. **Exhaustive Enumeration**: The runtime test iterates from -1 to 9, covering all valid codes (0-7) and illegal values.

2. **OpenACC Data Clauses**: Uses explicit partitioning with `gang`, `worker`, `vector` clauses in various combinations, including `gang(static:1)`, `worker(num:2)`, `vector(length:32)`.

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array `md_data[10][10][10]` with nested loops and different partitioning schemes.

4. **Template Metaprogramming**: C++ template specializations for each partition code 0-7 generate different OpenACC constructs at compile time.

5. **Volatile Control Flow**: Uses `volatile int selector` and `volatile int current_code` to prevent compiler optimizations from eliminating code paths.

6. **Redundant Clauses**: Includes `gang redundant` clause to trigger the "gang redundant" case (code 0).

7. **Final Reduction**: Performs a reduction sum to prevent dead code elimination.

**Compilation commands:**
