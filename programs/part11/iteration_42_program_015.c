This code appears to be from a C++ compiler's pretty-printer for OpenMP dependency clauses. It's handling the printing of different dependency types in OpenMP directives. Here's what each case does:

**Case breakdown:**

1. **`OMP_CLAUSE_DEPEND_IN`** → Prints `" update(in)"`
   - For `depend(in: ...)` clauses

2. **`OMP_CLAUSE_DEPEND_INOUT`** → Prints `" update(inout)"`
   - For `depend(inout: ...)` clauses

3. **`OMP_CLAUSE_DEPEND_OUT`** → Prints `" update(out)"`
   - For `depend(out: ...)` clauses

4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`** → Prints `" update(mutexinoutset)"`
   - For `depend(mutexinoutset: ...)` clauses (OpenMP 5.0+)

5. **`OMP_CLAUSE_DEPEND_INOUTSET`** → Prints `" update(inoutset)"`
   - For `depend(inoutset: ...)` clauses (OpenMP 5.0+)

6. **`OMP_CLAUSE_DEPEND_LAST`** → Prints `" destroy"`
   - For `depend(destroy: ...)` clauses (used with `omp_all_memory`)

**Context:**
This is likely from GCC's C++ frontend (`cp/` directory) where `pp_cxx_ws_string` is a pretty-printer function that outputs formatted text. The "update" prefix suggests this might be specifically for the `depend` clause in OpenMP `task` constructs with the `update` directive modifier.

**Example usage in OpenMP code:**
```cpp
#pragma omp task depend(in: x)      // Would print "update(in)"
#pragma omp task depend(out: y)     // Would print "update(out)"
#pragma omp task depend(inout: z)   // Would print "update(inout)"
```

The `default:` case with `break` handles any other dependency types that don't need special printing in this context.
