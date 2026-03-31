This code appears to be part of a C++ compiler's pretty-printer for OpenMP clauses, specifically handling different types of `depend` clauses. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → `update(in)`
   - For input dependencies (read-only access)

2. **`OMP_CLAUSE_DEPEND_INOUT`** → `update(inout)`
   - For input/output dependencies (read-write access)

3. **`OMP_CLAUSE_DEPEND_OUT`** → `update(out)`
   - For output dependencies (write-only access)

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → `update(mutexinoutset)`
   - For mutual exclusion dependencies in sets

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → `update(inoutset)`
   - For input/output dependencies in sets

6. **`OMP_CLAUSE_DEPEND_LAST`** → `destroy`
   - For destroy dependencies (likely for taskwait dependencies)

## Context:
This is likely from GCC's C++ frontend (`cp/` directory) where OpenMP constructs are being pretty-printed. The `pp_cxx_ws_string` function is adding whitespace and the string representation of the dependency type.

## Usage Example:
In OpenMP code, these would appear in task dependencies:
```cpp
#pragma omp task depend(in: x)      // OMP_CLAUSE_DEPEND_IN
#pragma omp task depend(inout: y)   // OMP_CLAUSE_DEPEND_INOUT  
#pragma omp task depend(out: z)     // OMP_CLAUSE_DEPEND_OUT
```

The `update(...)` syntax shown here might be for internal representation or a specific dialect, as standard OpenMP uses `depend(in: ...)`, not `depend(update(in): ...)`.
