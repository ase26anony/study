This test program is designed to trigger the pretty-printer for all uncovered lines:

1. **Covers all `update` modifier cases**: The code includes `depend` clauses with `in`, `inout`, `out`, `mutexinoutset`, and `inoutset` dependency types in various OpenMP constructs (`target data`, `target update`, `task`).

2. **Includes `destroy` clause**: The `DataManager::manage_data()` function and the task in `main()` use `depend(destroy: ...)`.

3. **Triggers compiler diagnostics**: 
   - Uses `volatile` variables to prevent optimization removal
   - Includes unused variables (`unused_in_target`, `unused_in_class`, `unused_in_lambda`) to trigger `-Wunused-variable` warnings
   - Uses template functions and lambdas to increase AST complexity

4. **Multiple contexts for robust coverage**:
   - Template function `test_depend_update_modifiers`
   - Lambda expression in `main()`
   - Class member function `DataManager::manage_data()`
   - Regular function `test_various_constructs()`

5. **Various OpenMP constructs**: Uses `target data`, `target update`, `task`, `taskwait`, and `parallel` regions to ensure the pretty-printer encounters `depend` clauses in different contexts.

**Compilation commands to trigger coverage**:

1. For diagnostic warnings (which will invoke the pretty-printer):
