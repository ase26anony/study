This code appears to be from the GCC compiler's pretty-printing module for OpenMP clauses, specifically handling the `depend` clause variants. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → `depend(in)`
   - Data dependency where the task reads from the specified storage location
   - Printed as `update(in)`

2. **`OMP_CLAUSE_DEPEND_INOUT`** → `depend(inout)`
   - Data dependency where the task reads from and writes to the specified storage location
   - Printed as `update(inout)`

3. **`OMP_CLAUSE_DEPEND_OUT`** → `depend(out)`
   - Data dependency where the task writes to the specified storage location
   - Printed as `update(out)`

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → `depend(mutexinoutset)`
   - Mutual exclusion dependency for tasks that update the same storage location
   - Printed as `update(mutexinoutset)`

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → `depend(inoutset)`
   - Multiple tasks can read the data, but only one can update it
   - Printed as `update(inoutset)`

6. **`OMP_CLAUSE_DEPEND_LAST`** → `depend(destroy)`
   - Used with `omp_alloctrait_t` to destroy an allocator
   - Printed as `destroy`

## Context:
This code is likely from `gcc/cp/cxx-pretty-print.cc` or a similar file in GCC's C++ frontend. The `pp_cxx_ws_string` function outputs whitespace followed by the specified string during pretty-printing of OpenMP constructs.

The `update(...)` terminology in the output strings suggests this might be related to the `depend` clause's `update` dependency type introduced in OpenMP 5.0, which is used for tasks that update a storage location.
