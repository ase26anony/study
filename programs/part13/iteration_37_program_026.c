This program comprehensively exercises the uncovered partition code mapping through:

1. **Exhaustive Partition Code Enumeration**: Tests codes -1 through 9, covering all valid (0-7) and illegal values.

2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination on `loop` directives with various partitioning schemes.

3. **Multi-dimensional Array Broadcasting**: Uses a 3D array with nested OpenACC regions and complex partitioning combinations.

4. **Template Metaprogramming**: C++ templates generate different partition combinations at compile-time, ensuring all code paths are instantiated.

5. **Volatile Control Flow**: Uses `volatile int` variables to prevent constant folding and ensure all branches remain in the generated code.

6. **Valid, Compilable Code**: Includes proper initialization, computation, and final reductions to prevent dead code elimination.

**Compilation commands for coverage testing:**
