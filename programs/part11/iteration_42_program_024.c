This code appears to be from a C++ compiler's pretty-printer for OpenMP clauses, specifically handling `depend` clause variants. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → `update(in)`
   - Input dependency: task depends on data being available for reading

2. **`OMP_CLAUSE_DEPEND_INOUT`** → `update(inout)`
   - Input/output dependency: task depends on data being available for both reading and writing

3. **`OMP_CLAUSE_DEPEND_OUT`** → `update(out)`
   - Output dependency: task depends on data being available for writing

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → `update(mutexinoutset)`
   - Mutual exclusion dependency set: tasks with this dependency can execute in any order but not concurrently

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → `update(inoutset)`
   - Input/output dependency set: similar to mutexinoutset but for read-write operations

6. **`OMP_CLAUSE_DEPEND_LAST`** → `destroy`
   - Destroy dependency: used with `depend` clause on `taskwait` or `taskgroup` constructs

## Context:
This is likely from GCC's C++ frontend (`cp/` directory) where the pretty-printer (`pp_cxx_*` functions) converts internal compiler representations back to human-readable OpenMP syntax. The `update` keyword is used in `depend` clauses for tasks that update data in a `mutexinoutset` or `inoutset` dependency type.

## Example Usage:
```cpp
#pragma omp task depend(inout: x)      // OMP_CLAUSE_DEPEND_INOUT
#pragma omp task depend(mutexinoutset: y) // OMP_CLAUSE_DEPEND_MUTEXINOUTSET
#pragma omp task depend(destroy: z)    // OMP_CLAUSE_DEPEND_LAST
```

The pretty-printer translates the internal enum values back to their corresponding OpenMP syntax strings when generating diagnostic messages or debugging output.
