This program is designed to:

1. **Exhaustively use all `depend` modifiers**: Each modifier (`in`, `out`, `inout`, `mutexinoutset`, `inoutset`, `destroy`) is used with valid lvalue expressions.

2. **Trigger pretty-printing via diagnostics**: 
   - Uses `[[deprecated]]` variable in `depend` clauses
   - Includes malformed clause (`undefined_var`) to trigger error paths
   - Compile with `-Wdeprecated-declarations` to see diagnostic output

3. **Utilize compiler dump flags**: The code structure ensures rich AST output with `-fdump-tree-original` and `-fdump-omp-all`.

4. **Combine with template and `constexpr`**: 
   - Template function `process_with_depend`
   - `conditional_omp_tasks` with `if constexpr`
   - Template specialization

5. **Structural diversity**:
   - Multiple namespaces and classes
   - Lambda expressions with OpenMP
   - `__attribute__((constructor))` function
   - Nested parallel regions and tasks

6. **Execution flow**:
   - Initializes shared data structures
   - Spawns tasks with all `depend` modifiers
   - Uses compile-time conditionals
   - Calculates checksum to prevent optimization removal

To maximize coverage of the target pretty-printer lines, compile with:
