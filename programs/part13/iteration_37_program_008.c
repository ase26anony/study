This program implements all the requested features:

1. **Exhaustive Partition Code Enumeration**: The runtime loop iterates from -1 to 9, covering all valid codes (0-7) and illegal values.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination with various partitioning strategies.

3. **Multi-dimensional Array Broadcasting**: Uses a 2D array with nested loops and complex data movement patterns.

4. **Template Metaprogramming**: The `PartitionGenerator` template is explicitly instantiated for all codes 0-7, ensuring compile-time generation of all partition patterns.

5. **Volatile Control Flow**: The `partition_selector` variable is declared volatile to prevent constant folding and ensure all code paths are generated.

6. **Minimal & Compilable**: Standalone C++ code with proper memory management and a final reduction to prevent optimization.

**Compilation commands:**
