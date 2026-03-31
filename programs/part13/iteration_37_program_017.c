This program comprehensively exercises the partition code mapping function by:

1. **Exhaustive Partition Code Enumeration**: Tests values from -1 to 9, covering all valid codes (0-7) and illegal values.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination on loop directives with both `parallel` and `kernels` regions.

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array with nested loops and different partitioning schemes, including `redundant` clauses.

4. **Template Metaprogramming**: C++ template specializations generate different partition combinations at compile-time for codes 0-7.

5. **Volatile Control Flow**: Uses `volatile int selector` to prevent constant folding and ensure all code paths are generated.

6. **Minimal & Compilable**: Standalone C++ code with proper initialization, computation, and output to prevent dead code elimination.

**Compilation commands:**
