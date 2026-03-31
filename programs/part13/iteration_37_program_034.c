This program implements all the requested features:

1. **Exhaustive Partition Code Enumeration**: Tests codes -1 through 9 in the runtime loop, covering all valid (0-7) and illegal values.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combinations like `gang worker`, `worker vector`, `gang worker vector`.

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array with complex nested loops and mixed partitioning schemes.

4. **Template Metaprogramming**: The `PartitionGenerator` template is explicitly instantiated for all codes 0-7, ensuring compile-time generation of all partition patterns.

5. **Volatile Control Flow**: Uses `volatile int partition_selector` to prevent constant folding and ensure all code paths remain viable.

6. **Minimal & Compilable**: Standalone C++17 code with proper memory management and final reductions to prevent dead code elimination.

**Compilation options to maximize coverage**:
