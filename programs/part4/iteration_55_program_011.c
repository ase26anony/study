This code appears to be part of a C++ compiler's pretty-printer for OpenMP clauses, specifically handling `depend` clause variants. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → `update(in)`
   - Dependence on input data (read-only)

2. **`OMP_CLAUSE_DEPEND_INOUT`** → `update(inout)`
   - Dependence on input/output data (read-write)

3. **`OMP_CLAUSE_DEPEND_OUT`** → `update(out)`
   - Dependence on output data (write-only)

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → `update(mutexinoutset)`
   - Mutual exclusion dependence for atomic operations

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → `update(inoutset)`
   - Set-based input/output dependence

6. **`OMP_CLAUSE_DEPEND_LAST`** → `destroy`
   - Dependence for destroying a dependency object

## Context:
This is likely from GCC's C++ frontend (`cp/` directory), specifically in the OpenMP clause pretty-printing code. The `pp_cxx_ws_string` function outputs formatted strings with appropriate whitespace for C++ pretty-printing.

## Example Usage:
In OpenMP code, these would appear in constructs like:
```cpp
#pragma omp task depend(in: x) depend(out: y)
{
    // Task code
}

#pragma omp task depend(mutexinoutset: lock_var)
{
    // Atomic task
}
```

The pretty-printer converts the internal compiler representations (`OMP_CLAUSE_DEPEND_*`) back to the textual OpenMP syntax that programmers would recognize.
