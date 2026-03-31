This program systematically exercises all partition codes through:

1. **Exhaustive Runtime Enumeration**: Tests partition codes -1 through 9 in a loop with volatile selectors
2. **Template Metaprogramming**: Compile-time generation of all 8 valid partition codes (0-7)
3. **Mixed OpenACC/OpenMP**: Uses both directive systems to stress compiler integration
4. **Multi-dimensional Arrays**: 3D arrays with complex nested loops
5. **All Partition Clauses**: Uses `gang`, `worker`, `vector`, `redundant`, and their combinations
6. **Volatile Control Flow**: Prevents compiler optimization from removing branches

**Compilation commands:**
