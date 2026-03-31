This program implements all the requested features:

1. **Exhaustive Partition Code Enumeration**: Tests codes -1 through 9 in the runtime loop, covering all valid codes (0-7) and illegal values.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination with `redundant` and `static:1` specifications.

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array `md_array[8][8][8]` with complex nested loops and data dependencies.

4. **Template Metaprogramming**: The `OpenACCGenerator` template is explicitly instantiated for codes 0-7 and illegal values (-1, 8), ensuring compile-time generation of all partition patterns.

5. **Volatile Control Flow**: Uses `volatile int partition_selector` and `volatile int outer_bound` to prevent constant folding and ensure all code paths are considered.

6. **Minimal & Compilable**: Standalone C++17 code with proper memory management and output to prevent dead code elimination.

**Compilation commands:**
