## Key Features for Coverage:

1. **Exhaustive `depend` Modifiers**: The program uses all six modifiers (`in`, `out`, `inout`, `mutexinoutset`, `inoutset`, `destroy`) across multiple contexts.

2. **Diagnostic Triggers**: 
   - `deprecated_var` marked with `[[deprecated]]` triggers warnings when used in `depend` clauses
   - Compile with `-Wdeprecated-declarations` to see diagnostic output

3. **Compiler Dump Compatibility**:
   - Use `-fdump-tree-original` to see pretty-printed OpenMP constructs
   - Use `-fdump-omp-all` for OpenMP-specific dumps
   - The nested structure ensures complex AST generation

4. **Template and `constexpr` Contexts**:
   - Template function `process_with_depend<T>`
   - Template class `TaskContainer<T>`
   - `if constexpr` in `conditional_omp_tasks`
   - Lambda expressions with OpenMP

5. **Structural Diversity**:
   - Global constructor with `__attribute__((constructor))`
   - Namespace `OpenMPTests`
   - Multiple functions with different OpenMP patterns
   - Array elements, pointers, and struct members as dependencies

6. **Edge Cases**:
   - Commented malformed `depend` clause (uncomment to test error paths)
   - Mixed dependency types in single directives
   - `taskwait` for synchronization

## Compilation Commands:
