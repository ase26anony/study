**Key features that trigger the uncovered pretty-printer paths:**

1. **All Dependency Modifiers**: The `switch` statement in `process_with_dependencies` creates tasks with `depend(in:)`, `depend(out:)`, `depend(inout:)`, `depend(mutexinoutset:)`, and `depend(inoutset:)` clauses.

2. **Destroy Dependency**: Explicit `depend(destroy: ...)` clause in both the template function and lambda.

3. **Nested Structures**: Tasks inside `taskgroup` with conditional creation paths.

4. **Array Sections**: `depend(inout: data[0:2])` and `depend(inout: buffer[0:N])` clauses.

5. **Template Context**: `process_with_dependencies` is a template function.

6. **Lambda Context**: `create_task_chain` is a lambda with OpenMP directives.

7. **Target Constructs**: `target_with_dependencies` uses `depend` clauses with `target` directives.

8. **Complex Dependency Chains**: The lambda creates a chain of tasks where each depends on the previous.

**Compilation commands to generate AST dumps:**
