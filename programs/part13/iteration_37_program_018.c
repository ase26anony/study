This program provides comprehensive coverage through:

1. **Exhaustive Partition Code Enumeration**: The `runtime_partition_test` function iterates over codes -1 through 9, covering all valid (0-7) and illegal values.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, and `vector` clauses individually and in combination on loop directives with various partitioning strategies.

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array `md_data[10][10][10]` with `copy` clauses inside nested parallel regions, including `redundant` reductions.

4. **Template Metaprogramming**: The `OpenACCGenerator` template is specialized for each partition code 0-7, ensuring all combinations are generated at compile-time.

5. **Volatile Control Flow**: The `selector` variable is declared `volatile` to prevent constant folding and ensure all code paths remain in the compiled output.

6. **Minimal & Compilable**: Standalone C++ code with proper `main()` function, array initialization, and final output to prevent dead code elimination.

**Compilation commands:**
