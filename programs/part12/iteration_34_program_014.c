This program systematically exercises all the uncovered `depend` clause modifiers:

1. **Exhaustive `depend` Clause Usage**: Contains `depend(in:)`, `depend(out:)`, `depend(inout:)`, `depend(mutexinoutset:)`, `depend(inoutset:)`, and `depend(destroy:)` clauses applied to various lvalue expressions (array elements, pointer dereferences, struct members).

2. **Triggers Pretty-Printing via Diagnostics**:
   - Uses `[[deprecated]]` variable `deprecated_var` in a `depend` clause to generate warnings
   - Conditional `#ifdef TRIGGER_ERROR` with undefined variable to trigger error diagnostics
   - Compile with `-Wdeprecated-declarations` to see warning output

3. **Utilizes Compiler Dump Flags**:
   - Complex nested OpenMP regions (tasks within parallel, taskgroups, master regions)
   - Multiple functions and scopes to generate rich dump output
   - Compile with `-fdump-tree-original` or `-fdump-omp-all` to see pretty-printed output

4. **Combines with Template and `constexpr` Contexts**:
   - Template function `process_with_depend` with specialization
   - `conditional_omp_tasks` using `if constexpr`
   - Lambda expression containing OpenMP task

5. **Structural Diversity**:
   - Namespace `OpenMPTests`
   - Class `TaskContainer` with static method
   - `__attribute__((constructor))` function
   - Multiple compilation units worth of complexity in one file

6. **Execution Flow**:
   - Initializes shared variables
   - Calls template functions with different depend types
   - Uses `destroy` clause for synchronization
   - Calculates checksum to ensure runtime execution

To maximize coverage of the pretty-printer lines, compile with:
