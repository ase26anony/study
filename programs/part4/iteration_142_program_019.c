This test program comprehensively covers all the required OpenMP dependency clause modifiers:

1. **All Dependency Modifiers**: Uses `depend(in:)`, `depend(out:)`, `depend(inout:)`, `depend(mutexinoutset:)`, `depend(inoutset:)`, and `depend(destroy:)` across multiple tasks.

2. **Nested Chains**: Creates dependency chains in loops and conditional blocks, with different modifiers in different scopes.

3. **Combined Constructs**: Uses `taskwait` with dependencies, `taskgroup`, and `target` constructs with `depend` clauses.

4. **Template and Lambda Contexts**: Places OpenMP directives within a template function and a lambda expression.

5. **Array Sections and Complex Dependents**: Uses array section notation (`arr[0:N/2]`) and pointer dereferencing (`*ptr`) in depend clauses.

To generate the compiler dumps that will exercise the pretty-printer code paths, compile with:
