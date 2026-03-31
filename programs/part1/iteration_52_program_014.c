**Explanation of Coverage Strategy:**

1. **All Depend Modifiers Covered:**
   - `depend(in: global_var)` - line with `OMP_CLAUSE_DEPEND_IN`
   - `depend(inout: member_var)` - line with `OMP_CLAUSE_DEPEND_INOUT`
   - `depend(out: heap_array[0])` - line with `OMP_CLAUSE_DEPEND_OUT`
   - `depend(mutexinoutset: heap_array[1])` - line with `OMP_CLAUSE_DEPEND_MUTEXINOUTSET`
   - `depend(inoutset: heap_array[2])` - line with `OMP_CLAUSE_DEPEND_INOUTSET`
   - `depend(destroy: heap_array[3])` - line with `OMP_CLAUSE_DEPEND_LAST`

2. **Complex Data Environment:**
   - Global variables (`global_var`)
   - Static global variables (`static_global_var`)
   - Class member variables (`member_var`)
   - Static class members (`static_member`)
   - Static local variables (`static_local`)
   - Heap-allocated pointers (`heap_array`)
   - References (`local_ref`)
   - Pointer dereferences (`*local_ptr`)
   - Array elements (`array[0]`)
   - Struct members (`data.x`)

3. **Nested and Compound Constructs:**
   - Tasks inside `parallel` regions
   - Tasks inside `single` constructs
   - `taskgroup` for synchronization
   - `taskwait` for dependency satisfaction
   - Combined with other clauses: `priority`, `mergeable`, `final`

4. **Triggering Pretty-Printing:**
   - The deliberate syntax error with `UndefinedType` will cause the compiler to emit an error
   - When compiled with `-fdump-tree-omp`, the OpenMP constructs will be dumped
   - The `-Werror=openmp-format` option may trigger additional diagnostics
   - The `-fdump-tree-all` option ensures maximum intermediate representation dumps

**Compilation Commands for Coverage:**
