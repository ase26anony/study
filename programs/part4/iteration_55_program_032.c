This code appears to be part of a C++ compiler's pretty-printing logic for OpenMP clauses, specifically handling `depend` clause variants. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → `depend(in: ...)`
   - Prints as `update(in)`
   - Input dependency: task depends on the listed variables being available for reading

2. **`OMP_CLAUSE_DEPEND_INOUT`** → `depend(inout: ...)`
   - Prints as `update(inout)`
   - Input/output dependency: task depends on exclusive access to the variables

3. **`OMP_CLAUSE_DEPEND_OUT`** → `depend(out: ...)`
   - Prints as `update(out)`
   - Output dependency: task depends on being able to write to the variables

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → `depend(mutexinoutset: ...)`
   - Prints as `update(mutexinoutset)`
   - Mutual exclusion dependency for sets of variables

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → `depend(inoutset: ...)`
   - Prints as `update(inoutset)`
   - Input/output set dependency

6. **`OMP_CLAUSE_DEPEND_LAST`** → `depend(...)`
   - Prints as `destroy`
   - Likely for `depend(destroy: ...)` clause used with `omp_interop_t` objects

## Context:
This is from GCC's C++ frontend pretty-printer (`cp/` directory). The `pp_cxx_ws_string` function outputs formatted text with appropriate whitespace. The `update(...)` syntax shown here might be for internal representation or debugging output, not the actual OpenMP syntax that would appear in source code.

The actual OpenMP syntax these correspond to would be:
```cpp
#pragma omp task depend(in: var1, var2)
#pragma omp task depend(inout: var3)
#pragma omp task depend(out: var4)
#pragma omp task depend(mutexinoutset: var5)
#pragma omp task depend(inoutset: var6)
#pragma omp task depend(destroy: interop_obj)
```
