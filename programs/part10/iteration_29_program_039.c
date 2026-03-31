This code appears to be part of a C/C++ compiler's pretty-printer for OpenMP clauses, specifically handling `depend` clause variants. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → Prints `update(in)`
   - For input dependencies (read-only access)

2. **`OMP_CLAUSE_DEPEND_INOUT`** → Prints `update(inout)`
   - For input/output dependencies (read-write access)

3. **`OMP_CLAUSE_DEPEND_OUT`** → Prints `update(out)`
   - For output dependencies (write-only access)

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → Prints `update(mutexinoutset)`
   - For mutual exclusion dependencies in sets

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → Prints `update(inoutset)`
   - For input/output dependencies in sets

6. **`OMP_CLAUSE_DEPEND_LAST`** → Prints `destroy`
   - For dependency destruction (likely for `depend` clause with `destroy` modifier)

## Context:
This is likely from GCC's C++ frontend (`cp/` directory) where `pp_cxx_ws_string` is a pretty-printer function that outputs formatted text. The code handles the pretty-printing of OpenMP directives when generating diagnostic messages or when outputting preprocessed code.

## Example Usage:
In OpenMP, these would be used like:
```cpp
#pragma omp task depend(in: x)      // OMP_CLAUSE_DEPEND_IN
#pragma omp task depend(out: y)     // OMP_CLAUSE_DEPEND_OUT  
#pragma omp task depend(inout: z)   // OMP_CLAUSE_DEPEND_INOUT
```

The `update(...)` syntax shown here might be for internal representation or a specific output format, as the actual OpenMP syntax uses `depend(in: ...)`, not `update(in)`.
