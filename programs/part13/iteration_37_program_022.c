This program provides multiple pathways to exercise the partition code mapping function:

1. **Exhaustive Enumeration**: The main loop tests partition codes from -1 to 9, covering all valid codes (0-7) and illegal values.

2. **OpenACC Partitioning Clauses**: Uses `gang`, `worker`, `vector`, `redundant`, and combinations thereof in various loop directives.

3. **Multi-dimensional Broadcasting**: Uses 3D array indexing patterns that require broadcast operations across different partitioning schemes.

4. **Template Metaprogramming**: The `OpenACCPartitionGenerator` template is explicitly instantiated for all codes 0-7, ensuring compile-time generation of all partition combinations.

5. **Volatile Control Flow**: Uses `volatile int` variables to prevent constant folding and ensure all code paths are generated.

6. **Complex Nesting**: Includes deeply nested loops with mixed partitioning schemes to stress the compiler's internal representation.

**Compilation commands**:
