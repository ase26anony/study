This test program covers all the requirements:

1. **All Dependency Modifiers**: Uses `in`, `out`, `inout`, `mutexinoutset`, `inoutset`, and `destroy` modifiers in various contexts.

2. **Complex Dependency Graphs**: Creates tasks in loops with different modifiers, nested taskgroups, and conditional blocks.

3. **Combined OpenMP Constructs**: Uses `taskwait` with dependencies, `taskgroup`, and `target` constructs with `depend` clauses.

4. **Template and Lambda Contexts**: Places OpenMP directives within a template function and lambda expressions.

5. **Array Sections and Complex Dependents**: Uses array section notation (`arr[0:50]`) and pointer dereferences (`*ptr`) in depend clauses.

To compile and generate the AST dumps that will exercise the pretty-printer:
