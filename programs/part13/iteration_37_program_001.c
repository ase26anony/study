This program comprehensively exercises the partition code mapping function by:

1. **Exhaustive Partition Code Enumeration**: Tests codes from -1 to 9, covering all valid (0-7) and illegal values.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination on loop directives.

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array with complex nested loops and `copy`/`present_or_copy` clauses.

4. **Template Metaprogramming**: The `PartitionGenerator` template is explicitly instantiated for all partition codes 0-7, ensuring compile-time generation of different OpenACC structures.

5. **Volatile Control Flow**: Uses `volatile int` variables to prevent constant folding and ensure all code paths are generated.

6. **Multiple OpenACC Constructs**: Uses both `parallel` and `kernels` regions with different partitioning schemes.

7. **Reduction Operations**: Final reductions prevent dead code elimination and ensure all computations are performed.

**Compilation commands:**
