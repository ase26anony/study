This program implements all the requested features:

1. **Exhaustive Partition Code Enumeration**: Tests codes -1 through 9 (covering 0-7 and illegal values)
2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination
3. **Multi-dimensional Array Broadcasting**: Uses a 3D array with nested OpenACC loops
4. **Template Metaprogramming**: C++ template specializations for each partition code 0-7
5. **Volatile Control Flow**: Uses `volatile int` to prevent constant folding and optimization
6. **Minimal & Compilable**: Standalone C++ with proper OpenACC pragmas

**Compilation commands:**
