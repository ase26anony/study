This code appears to be part of a C++ compiler's pretty-printer for OpenMP clauses, specifically handling `depend` clause variants. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → `update(in)`
   - Input dependency: task depends on data being available for reading

2. **`OMP_CLAUSE_DEPEND_INOUT`** → `update(inout)`
   - Input/output dependency: task depends on data being available for both reading and writing

3. **`OMP_CLAUSE_DEPEND_OUT`** → `update(out)`
   - Output dependency: task depends on data being available for writing

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → `update(mutexinoutset)`
   - Mutual exclusion dependency: tasks with this dependency on the same data cannot execute concurrently

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → `update(inoutset)`
   - Similar to mutexinoutset but allows concurrent reads

6. **`OMP_CLAUSE_DEPEND_LAST`** → `destroy`
   - Special dependency type for task destruction

## Context:
This is likely from GCC's C++ frontend (`cp/` directory) where `pp_cxx_ws_string` is a pretty-printing function that outputs formatted text. The code handles how OpenMP directives are displayed when pretty-printing ASTs or generating diagnostic messages.

The `update(...)` syntax shown here is for the `depend` clause when used with the `update` directive in OpenMP, not to be confused with the general `depend` clause syntax for tasks.
