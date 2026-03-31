## Key Features:

1. **Exhaustive `depend` Clause Usage**: The program uses all modifier types:
   - `depend(in: ...)` - lines 30, 64, 91
   - `depend(out: ...)` - lines 35, 90
   - `depend(inout: ...)` - line 40
   - `depend(mutexinoutset: ...)` - lines 49, 108
   - `depend(inoutset: ...)` - lines 60, 111
   - `depend(destroy: ...)` - lines 73, 124

2. **Diagnostic Triggers**:
   - Uses `[[deprecated]]` variable with `depend(out: deprecated_var)` (line 35)
   - Malformed `depend` clause in `malformed_depend()` function (line 81)
   - Compile with `-Wdeprecated-declarations` to see warnings

3. **Compiler Dump Compatibility**:
   - Complex nested OpenMP regions (parallel → single → task → parallel for → task)
   - Multiple functions and classes increase AST complexity
   - Use `-fdump-tree-original` or `-fdump-omp-all` to see pretty-printed output

4. **Template and `constexpr` Contexts**:
   - Template function `process_with_depend` (line 26)
   - Template specialization `TaskGenerator` with `if constexpr` (line 55)
   - Template method in class (line 107)

5. **Structural Diversity**:
   - Global namespace functions
   - `OpenMPTests` namespace
   - `OpenMPContainer` class with methods
   - Lambda expression (line 64)
   - Constructor function with `__attribute__((constructor))` (line 70)

6. **Execution Flow**:
   - Initializes shared data structures
   - Calls multiple functions with different `depend` clause types
   - Uses `taskwait` for synchronization
   - Calculates and prints checksum to ensure execution

## Compilation Commands:
