This program implements all the requested features:

1. **Exhaustive Partition Code Enumeration**: Tests codes -1 through 9 in the runtime test, covering all valid codes (0-7) and illegal values.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combinations like `gang worker`, `gang vector`, `worker vector`, and `gang worker vector`.

3. **Multi-dimensional Array Broadcasting**: Uses `md_data[8][8][8]` with `copy` clauses and accesses it from different partition patterns.

4. **Template Metaprogramming**: The `PartitionGenerator` template is specialized for each code 0-7, ensuring compile-time generation of all valid partition patterns.

5. **Volatile Control Flow**: Uses `volatile int selector` and `volatile int current_code` to prevent constant folding and ensure all code paths remain in the compiled binary.

6. **Minimal & Compilable**: Standalone C++ code with proper memory management and a final reduction to prevent optimization removal.

**Compilation commands:**
