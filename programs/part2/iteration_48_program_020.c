This test program is designed to trigger the uncovered pretty-printer lines through several mechanisms:

1. **Multiple OpenMP constructs**: Uses `target data`, `target update`, `target enter/exit data`, `task`, and `taskwait` constructs with `depend` clauses.

2. **All `update` modifiers**: Covers `update(in)`, `update(inout)`, `update(out)`, `update(mutexinoutset)`, and `update(inoutset)`.

3. **`destroy` clause**: Includes `depend(destroy: ...)` in `taskwait` and `target enter/exit data` constructs.

4. **Compiler diagnostics**: Uses `volatile` variables that trigger `-Wunused-variable` warnings, causing GCC to print the OpenMP constructs in diagnostic messages.

5. **Various contexts**: Template functions, lambdas, class member functions, and regular functions to ensure the pretty-printer is invoked in different AST contexts.

6. **`declare target`**: Ensures variables are valid for OpenMP target offloading contexts.

To maximize coverage of the uncovered lines, compile with:
