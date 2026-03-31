This code appears to be from a C++ compiler's pretty-printing module, specifically handling OpenMP dependency clauses. Here's what each case does:

1. **OMP_CLAUSE_DEPEND_IN**: Prints `" update(in)"` - for input dependencies
2. **OMP_CLAUSE_DEPEND_INOUT**: Prints `" update(inout)"` - for input/output dependencies  
3. **OMP_CLAUSE_DEPEND_OUT**: Prints `" update(out)"` - for output dependencies
4. **OMP_CLAUSE_DEPEND_MUTEXINOUTSET**: Prints `" update(mutexinoutset)"` - for mutual exclusion dependencies
5. **OMP_CLAUSE_DEPEND_INOUTSET**: Prints `" update(inoutset)"` - for set-based input/output dependencies
6. **OMP_CLAUSE_DEPEND_LAST**: Prints `" destroy"` - for destroy dependencies

The `pp_cxx_ws_string` function likely outputs the string with appropriate whitespace formatting. The `default` case handles any unexpected dependency types by doing nothing.

These clauses are used in OpenMP directives like:
```cpp
#pragma omp task depend(in: x) depend(out: y)
```

The code suggests this is part of a C++ front-end that needs to pretty-print OpenMP constructs for debugging or diagnostic output.
