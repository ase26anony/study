This code appears to be part of a C++ pretty printer for OpenMP clauses, specifically handling different types of `depend` clauses. Here's what each case represents:

## OpenMP `depend` Clause Types:

1. **`OMP_CLAUSE_DEPEND_IN`** → `update(in)`
   - Data is only read, not modified

2. **`OMP_CLAUSE_DEPEND_INOUT`** → `update(inout)`
   - Data is both read and modified

3. **`OMP_CLAUSE_DEPEND_OUT`** → `update(out)`
   - Data is only written, not read

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → `update(mutexinoutset)`
   - For mutual exclusion with `inoutset` semantics

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → `update(inoutset)`
   - For reduction-like operations where order doesn't matter

6. **`OMP_CLAUSE_DEPEND_LAST`** → `destroy`
   - For destroying dependencies (likely for `depend` objects)

## Context:
This is likely from GCC's or another compiler's source code that handles pretty-printing OpenMP directives. The `pp_cxx_ws_string` function prints the string with appropriate whitespace formatting for C++ output.

## Example Usage:
In OpenMP, these would be used like:
```cpp
#pragma omp task depend(in: x)      // depends on x being available for reading
#pragma omp task depend(out: y)     // produces y
#pragma omp task depend(inout: z)   // both reads and writes z
```

The `update` form shown here is likely for `depend` clauses used with `update` directives or for pretty-printing purposes.
