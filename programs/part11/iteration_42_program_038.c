This code appears to be part of a C/C++ compiler's pretty-printer for OpenMP clauses, specifically handling the `depend` clause variants. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → Prints `update(in)`
   - For input dependencies (read-only)

2. **`OMP_CLAUSE_DEPEND_INOUT`** → Prints `update(inout)`
   - For input/output dependencies (read-write)

3. **`OMP_CLAUSE_DEPEND_OUT`** → Prints `update(out)`
   - For output dependencies (write-only)

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → Prints `update(mutexinoutset)`
   - For mutual exclusion inoutset dependencies (OpenMP 5.0+)

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → Prints `update(inoutset)`
   - For inoutset dependencies (OpenMP 5.0+)

6. **`OMP_CLAUSE_DEPEND_LAST`** → Prints `destroy`
   - For `depend(destroy:...)` clause (OpenMP 5.0+)

## Context:
This is likely from GCC's C++ frontend (`cp/` directory) where `pp_cxx_ws_string` is a pretty-printing function that outputs formatted text. The code handles the pretty-printing of OpenMP directives when generating diagnostic messages or when outputting preprocessed code.

## Example Usage:
When the compiler encounters:
```cpp
#pragma omp task depend(in: x)
```
It would use this code to pretty-print the `depend(in: x)` clause as `update(in)` in certain output contexts.

The `default:` case handles any other `depend` clause types that don't need special pretty-printing or are handled elsewhere.
