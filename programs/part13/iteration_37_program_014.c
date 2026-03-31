This program comprehensively exercises the partition code mapping function by:

1. **Exhaustive Partition Code Enumeration**: Tests codes -1 through 9, covering all valid (0-7) and illegal values
2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination
3. **Multi-dimensional Array Broadcasting**: Uses a 3D array with complex nested loops and different partitioning schemes
4. **Template Metaprogramming**: C++ templates generate different OpenACC patterns for each partition code at compile time
5. **Volatile Control Flow**: Uses `volatile` variables to prevent constant folding and optimization
6. **Multiple OpenACC Constructs**: Uses both `parallel` and `kernels` regions with various loop directives

**Compilation commands:**
