This test program comprehensively exercises the uncovered pretty-printer lines by:

1. **Using all dependency modifiers**: `in`, `out`, `inout`, `mutexinoutset`, `inoutset`, and `destroy` are all present in various task constructs.

2. **Creating nested dependency chains**: Tasks are created in loops, conditional blocks, and nested within other tasks, forming complex dependency graphs.

3. **Combining OpenMP constructs**: Uses `taskwait` with dependencies, `taskgroup`, and includes a `target` construct with dependencies (conditionally compiled).

4. **Template and lambda contexts**: The main logic is in a template function, and additional tasks are created within a lambda expression.

5. **Array sections and pointers**: Uses array section notation (`arr[0:n]`) and pointer dereferences (`*ptr`) as dependents.

6. **Multiple execution contexts**: Tasks are created in `main()`, in a template function, and within a lambda, ensuring the pretty-printer encounters these clauses in different AST contexts.

To generate the coverage data, compile with:
