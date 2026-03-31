This program comprehensively exercises the partition code mapping function by:

1. **Exhaustive Partition Code Enumeration**: Tests values from -1 to 9 (covering 0-7 and illegal values)
2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination
3. **Multi-dimensional Array Broadcasting**: Uses 3D arrays with complex nested partitioning
4. **Template Metaprogramming**: C++ templates generate specialized code for each partition code 0-7
5. **Volatile Control Flow**: Uses `volatile int` to prevent constant folding and ensure all code paths are generated
6. **Minimal & Compilable**: Standalone C++ code with proper `main()` function

**Compilation commands:**
