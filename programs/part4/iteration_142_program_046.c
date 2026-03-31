This test program includes:

1. **All Dependency Modifiers**: Uses `depend(in:)`, `depend(out:)`, `depend(inout:)`, `depend(mutexinoutset:)`, `depend(inoutset:)`, and `depend(destroy:)` across different tasks.

2. **Template Context**: The `process_with_dependencies` function is templated, ensuring the pretty-printer handles dependencies in generic contexts.

3. **Lambda Expressions**: Uses a lambda function `create_task_lambda` that creates tasks with different dependency modifiers based on input strings.

4. **Array Sections**: Includes `depend(in: arr[0:N])` and `depend(inout: arr[0:N/2])` with array section notation.

5. **Pointer Dependencies**: Uses `depend(out: *ptr)` and other modifiers with pointer-based dependents.

6. **Complex Dependency Graphs**: Creates chains of tasks with dependencies, uses `taskwait` with dependencies, and nests tasks within `taskgroup`.

7. **Conditional Contexts**: Places `depend(destroy:)` clauses inside conditional blocks to ensure they're encountered in different AST paths.

8. **Target Construct**: Includes a `target` directive with `depend` clause to test that context as well.

**Compilation commands for coverage testing:**
