This program systematically exercises all partition codes through:

1. **Exhaustive Partition Code Enumeration**: Tests codes -1 through 9 in a loop
2. **OpenACC Data Clauses with Explicit Partitioning**: Uses `gang`, `worker`, `vector` clauses individually and in combination
3. **Multi-dimensional Array Broadcasting**: Uses a 3D array with complex nested loops
4. **Template Metaprogramming**: Generates different partition combinations at compile-time
5. **Volatile Control Flow**: Uses `volatile int partition_selector` to prevent optimization
6. **Redundant Clauses**: Includes `gang redundant` for partition code 0

Compile with:
