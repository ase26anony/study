This program implements all the requested features:

1. **Exhaustive Partition Code Enumeration**: Tests codes -1 through 9 (covering 0-7 and illegal values)
2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination
3. **Multi-dimensional Array Broadcasting**: Uses 3D array with complex nested loops
4. **Template Metaprogramming**: C++ templates generate different partition combinations at compile time
5. **Volatile Control Flow**: `volatile int partition_selector` prevents constant folding
6. **Compilable Code**: Standalone C++ with proper OpenACC pragmas

**Compilation commands:**
