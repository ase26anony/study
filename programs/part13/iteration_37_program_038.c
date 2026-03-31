This program comprehensively exercises the uncovered partition code mapping by:

1. **Exhaustive Enumeration**: Tests partition codes -1 through 9 (covering 0-7 valid codes and illegal values)
2. **OpenACC Data Clauses**: Uses `copy`, `copyin`, and explicit partitioning with `gang`, `worker`, `vector` clauses
3. **Multi-dimensional Array Broadcasting**: Uses 3D array with complex nested loops
4. **Template Metaprogramming**: C++ templates generate specialized OpenACC code for each partition code 0-7
5. **Volatile Control Flow**: Uses `volatile int selector` to prevent constant folding and ensure all code paths are generated
6. **Mixed Partitioning**: Combines redundant and partitioned clauses in the same constructs
7. **Multiple OpenACC Constructs**: Uses both `parallel` and `kernels` regions

**Compilation commands:**
