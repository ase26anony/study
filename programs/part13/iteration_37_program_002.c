This program implements all the requested features:

1. **Exhaustive Partition Code Enumeration**: Tests values 0-7 and illegal values (-1, 8) through both compile-time templates and runtime loops.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination with various parameters (`static:1`, `static:2`, `(32)`, `(128)`).

3. **Multi-dimensional Array Broadcasting**: Uses 3D array access patterns and nested loops with different partitioning schemes.

4. **Template Metaprogramming**: The `PartitionGenerator` template generates different OpenACC patterns for each partition code at compile time using `if constexpr`.

5. **Volatile Control Flow**: Uses `volatile int partition_code` to prevent constant folding and ensure all code paths are generated.

6. **Minimal & Compilable**: Standalone C++17 code with proper memory management and a final reduction to prevent optimization.

**Compilation commands to test different paths:**
