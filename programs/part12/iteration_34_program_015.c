## Key Features of This Program:

1. **Exhaustive `depend` Clause Usage**: Contains all six modifier types from the uncovered block:
   - `depend(in: ...)` - lines 30-31
   - `depend(out: ...)` - lines 37-38  
   - `depend(inout: ...)` - lines 44-45
   - `depend(mutexinoutset: ...)` - lines 50-51
   - `depend(inoutset: ...)` - lines 56-57
   - `depend(destroy: ...)` - lines 62-63

2. **Diagnostic Triggering**: Uses `[[deprecated]]` variable (line 8) within a `depend` clause (line 85) to generate warning messages that invoke the pretty-printer.

3. **Compiler Dump Compatibility**: Complex nested OpenMP regions (parallel → single → multiple tasks) ensure rich AST structure for `-fdump-tree-original` and `-fdump-omp-all` outputs.

4. **Template and `constexpr` Contexts**: Template function `process_with_depend` (line 21) with `if constexpr` ensures template instantiation. Template specializations (lines 115-145) test different compilation phases.

5. **Structural Diversity**: Uses namespaces, classes with constructor attributes, lambda expressions, and multiple functions to test the pretty-printer across different scopes.

6. **Edge Cases**: Commented malformed `depend` clause (line 79) can be uncommented to test error reporting paths.

## Recommended Compilation Commands:
