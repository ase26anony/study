**Explanation of Coverage Strategy:**

1. **All `depend` Modifiers Covered:**
   - `depend(in: global_var)` - Line 2154 (`OMP_CLAUSE_DEPEND_IN`)
   - `depend(inout: member_var)` - Line 2157 (`OMP_CLAUSE_DEPEND_INOUT`)
   - `depend(out: global_data.x)` - Line 2160 (`OMP_CLAUSE_DEPEND_OUT`)
   - `depend(mutexinoutset: set_array[0])` - Line 2163 (`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`)
   - `depend(inoutset: set_array[2])` - Line 2166 (`OMP_CLAUSE_DEPEND_INOUTSET`)
   - `depend(destroy: heap_array[0])` - Line 2169 (`OMP_CLAUSE_DEPEND_LAST`)

2. **Complex Data Environment:**
   - Global variables (`global_var`)
   - Static variables (`static_global`, `static_local`)
   - Heap-allocated pointers (`heap_array`, `local_ptr`)
   - Class member variables (`member_var`)
   - References and dereferenced pointers (`*ptr`)
   - Array elements (`set_array[0]`)
   - Struct members (`global_data.x`)

3. **Nested and Compound Constructs:**
   - Tasks inside `parallel` region
   - `taskgroup` with dependent tasks
   - `taskwait` for synchronization
   - Combined with `priority`, `final`, `mergeable` clauses

4. **Triggering Pretty-Printing:**
   - The `problematic_function()` contains an `UndeclaredType` syntax error
   - When GCC encounters this, it may dump the surrounding context including OpenMP constructs
   - Compilation with `-fdump-tree-omp` will explicitly invoke the pretty-printer
   - The `-Werror=openmp-format` option may trigger format checking of the clauses

**Compilation Commands for Coverage:**
