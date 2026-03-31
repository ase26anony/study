Looking at this code snippet, it appears to be part of a C/C++ compiler's pretty-printer for OpenMP clauses, specifically handling `depend` clause types. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → `update(in)`
   - Read-only dependency

2. **`OMP_CLAUSE_DEPEND_INOUT`** → `update(inout)`
   - Read-write dependency

3. **`OMP_CLAUSE_DEPEND_OUT`** → `update(out)`
   - Write-only dependency

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → `update(mutexinoutset)`
   - Mutual exclusion dependency for atomic operations

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → `update(inoutset)`
   - Set-based dependency (OpenMP 5.0+)

6. **`OMP_CLAUSE_DEPEND_LAST`** → `destroy`
   - Used for `depend(destroy: ...)` clause to destroy dependency objects

## Context:
This code is likely from GCC's C++ frontend (`cp/` directory) or similar compiler infrastructure. The `pp_cxx_ws_string` function prints OpenMP clause strings with appropriate whitespace formatting for pretty-printing source code.

## Example Usage:
In OpenMP source code, these would appear as:
```cpp
#pragma omp task depend(in: x)          // OMP_CLAUSE_DEPEND_IN
#pragma omp task depend(inout: y)       // OMP_CLAUSE_DEPEND_INOUT  
#pragma omp task depend(out: z)         // OMP_CLAUSE_DEPEND_OUT
#pragma omp task depend(mutexinoutset: m) // OMP_CLAUSE_DEPEND_MUTEXINOUTSET
```

The `default:` case handles any other dependency types that don't need special pretty-printing.
