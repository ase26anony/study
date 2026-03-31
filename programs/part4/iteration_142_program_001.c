**Key features that trigger the uncovered pretty-printer paths:**

1. **All Dependency Modifiers**: The code uses `depend(in:)`, `depend(out:)`, `depend(inout:)`, `depend(mutexinoutset:)`, `depend(inoutset:)`, and `depend(destroy:)` across different contexts.

2. **Nested and Complex Dependency Chains**: Tasks are created in loops with switches, inside conditionals, within taskgroups, and inside lambda expressions.

3. **Multiple OpenMP Constructs**: Uses `task`, `taskwait`, `taskgroup`, and `target` constructs with dependency clauses.

4. **Template Context**: The `process_data` function is templated, ensuring the pretty-printer handles dependencies in generic contexts.

5. **Array Sections and Pointers**: Uses array section notation (`arr[0:N]`) and pointer dereferences (`*ptr`) as dependents.

6. **Mixed Scopes**: Dependencies appear in parallel regions, single regions, and nested scopes.

**Compilation commands to generate dumps that invoke the pretty-printer:**
