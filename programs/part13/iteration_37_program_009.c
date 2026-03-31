This program comprehensively addresses all requirements:

1. **Exhaustive Enumeration**: Tests partition codes -1 through 9 in runtime loop, covering all valid (0-7) and illegal values.

2. **OpenACC Data Clauses**: Uses `copy` clauses with multi-dimensional arrays and explicit partitioning via `gang`, `worker`, `vector` clauses in various combinations.

3. **Multi-dimensional Broadcasting**: Uses 3D array `multi_dim[10][10][10]` with complex nested loops and mixed partitioning.

4. **Template Metaprogramming**: `PartitionGenerator` template with specializations for codes 0-7, instantiated at compile-time.

5. **Volatile Control Flow**: Uses `volatile int selector` to prevent constant folding and ensure all switch branches are compiled.

6. **Minimal & Compilable**: Complete standalone C++ program with `main()` function, proper includes, and final output.

**Compilation commands:**
