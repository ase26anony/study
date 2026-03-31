This program comprehensively exercises the uncovered code by:

1. **Exhaustive Partition Code Enumeration**: Tests codes -1 through 9 in `runtime_partition_test()`, covering all valid codes (0-7) and illegal values.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, and `vector` clauses individually and in combination across different branches.

3. **Multi-dimensional Array Broadcasting**: Uses `md_array[10][10][10]` with `copy` clauses and nested loops with different partitioning schemes.

4. **Template Metaprogramming**: The `OpenACCPartitionGenerator` template is instantiated for all codes 0-7, generating different OpenACC loop structures at compile time.

5. **Volatile Control Flow**: Uses `volatile int partition_code` to prevent constant folding and ensure all code paths are considered by the compiler.

6. **Minimal & Compilable**: Standalone C++ code with proper initialization, computation, and output to prevent dead code elimination.

**Compilation commands:**
