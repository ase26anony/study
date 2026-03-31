This code appears to be part of a C++ compiler's pretty-printer for OpenMP clauses, specifically handling `depend` clause variants. Here's what each case represents:

## OpenMP `depend` Clause Types

1. **`OMP_CLAUSE_DEPEND_IN`** → `update(in)`
   - Represents input dependency (read-only)

2. **`OMP_CLAUSE_DEPEND_INOUT`** → `update(inout)`
   - Represents input/output dependency (read-write)

3. **`OMP_CLAUSE_DEPEND_OUT`** → `update(out)`
   - Represents output dependency (write-only)

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → `update(mutexinoutset)`
   - Represents mutual exclusion dependency for atomic operations

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → `update(inoutset)`
   - Represents set-based input/output dependency

6. **`OMP_CLAUSE_DEPEND_LAST`** → `destroy`
   - Represents dependency for destroying a resource

## Context
This is likely from GCC's C++ frontend (`cp/` directory), where `pp_cxx_ws_string` is a pretty-printing function that outputs formatted text. The code handles how different OpenMP dependency types are displayed when pretty-printing the AST or generating diagnostics.

## Example Usage
In OpenMP code, these would appear in constructs like:
```cpp
#pragma omp task depend(in: x) depend(out: y)
#pragma omp task depend(mutexinoutset: lock_var)
```

The pretty-printer converts internal compiler representations (`OMP_CLAUSE_DEPEND_*`) back to their textual OpenMP syntax for display purposes.
