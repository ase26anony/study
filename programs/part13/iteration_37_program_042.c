This program provides comprehensive coverage through:

1. **Exhaustive Partition Code Enumeration**: Tests values -1 through 9 in the runtime test, covering all valid codes (0-7) and illegal values.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination on loop directives with various partition strategies.

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array `data[100][100][100]` with nested parallel regions and copy clauses.

4. **Template Metaprogramming**: The `PartitionGenerator` template is specialized for each partition code 0-7, ensuring all are instantiated at compile time.

5. **Volatile Control Flow**: The `selector` variable is volatile, preventing constant folding and ensuring all code paths remain in the compiled output.

6. **Minimal & Compilable**: Standalone C++ code with proper `main()` function, array initialization, and result computation to prevent dead code elimination.

**Compilation commands:**
