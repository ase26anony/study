This code appears to be from a C++ compiler's pretty-printer implementation, specifically handling OpenMP dependency clauses. Here's what each case does:

**Code Explanation:**

1. **`OMP_CLAUSE_DEPEND_IN`**: Prints `" update(in)"` - for input dependencies
2. **`OMP_CLAUSE_DEPEND_INOUT`**: Prints `" update(inout)"` - for input/output dependencies  
3. **`OMP_CLAUSE_DEPEND_OUT`**: Prints `" update(out)"` - for output dependencies
4. **`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`**: Prints `" update(mutexinoutset)"` - for mutual exclusion dependencies
5. **`OMP_CLAUSE_DEPEND_INOUTSET`**: Prints `" update(inoutset)"` - for set-based dependencies
6. **`OMP_CLAUSE_DEPEND_LAST`**: Prints `" destroy"` - for dependency destruction
7. **`default`**: Does nothing (handles any unexpected cases)

**Context:**
This is likely part of a compiler's OpenMP implementation where dependency clauses are being pretty-printed for debugging or diagnostic output. The `pp_cxx_ws_string` function probably adds whitespace and then prints the specified string.

**OpenMP Dependency Types:**
- `in`: Task depends on the variable being available as input
- `out`: Task produces the variable as output
- `inout`: Task both reads and modifies the variable
- `mutexinoutset`: For mutual exclusion between tasks accessing the same variable
- `inoutset`: For set-based dependencies
- `destroy`: For destroying dependency objects

The code shows how different OpenMP dependency semantics are mapped to their textual representations during code generation or debugging output.
