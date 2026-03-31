**Key features that trigger the uncovered pretty-printer paths:**

1. **All Dependency Modifiers**: The switch statement creates tasks with `depend(in:)`, `depend(out:)`, `depend(inout:)`, `depend(mutexinoutset:)`, and `depend(inoutset:)`.

2. **Destroy Dependency**: The conditional block contains `#pragma omp task depend(destroy: data[0])` to trigger `OMP_CLAUSE_DEPEND_LAST`.

3. **Template Context**: The function is templated, ensuring the pretty-printer handles clauses within generic contexts.

4. **Lambda Expression**: Contains an OpenMP task inside a lambda, testing nested scopes.

5. **Array Sections**: Uses `data[i:i+2]` notation in dependencies.

6. **Pointer Dependencies**: The overloaded version uses `*ptr` and `ptr[1]` as dependents.

7. **Target Constructs**: Includes `target` directives with `depend` clauses using array sections.

8. **Taskwait with Dependencies**: Uses `#pragma omp taskwait depend(inout: ...)`.

9. **Taskgroup**: Contains nested tasks with dependencies.

**Compilation commands to generate dumps:**
