This program provides comprehensive coverage through:

1. **Exhaustive Partition Code Enumeration**: Tests values -1 through 9 (covering 0-7 valid codes and illegal values)
2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination
3. **Multi-dimensional Array Broadcasting**: Includes 2D and 3D arrays with complex nested loops
4. **Template Metaprogramming**: C++ templates generate specialized code for each partition code 0-7
5. **Volatile Control Flow**: Uses `volatile int` to prevent constant folding and ensure all branches are compiled
6. **Minimal & Compilable**: Standalone C++ with proper OpenACC pragmas

**Compilation commands:**
