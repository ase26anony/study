**Explanation of Coverage Strategy:**

1. **All `depend` Modifiers Covered:**
   - `depend(in: global_var)` - line 2154 (`OMP_CLAUSE_DEPEND_IN`)
   - `depend(inout: global_var)` - line 2157 (`OMP_CLAUSE_DEPEND_INOUT`)
   - `depend(out: static_global_var)` - line 2160 (`OMP_CLAUSE_DEPEND_OUT`)
   - `depend(mutexinoutset: array[1])` - line 2163 (`OMP_CLAUSE_DEPEND_MUTEXINOUTSET`)
   - `depend(inoutset: *ptr_array[2])` - line 2166 (`OMP_CLAUSE_DEPEND_INOUTSET`)
   - `depend(destroy: *heap_ptr)` - line 2169 (`OMP_CLAUSE_DEPEND_LAST`)

2. **Complex Data Environment:**
   - Global variables (`global_var`)
   - Static variables (`static_global_var`, `static_local`)
   - Heap-allocated pointers (`heap_ptr`, `local_heap`)
   - Array elements (`array[1]`, `ptr_array[2]`)
   - Class members (`member_var`, `static_member`)
   - References (`ref`)

3. **Nested and Compound Constructs:**
   - Tasks inside `parallel` region
   - `taskgroup` with dependency chains
   - Tasks in member functions
   - Combined with other clauses (`priority`, `final`, `mergeable`)

4. **Triggering Pretty-Printing:**
   - The `problematic_function()` contains a deliberate syntax error (`UndeclaredType`)
   - When compiled, this will trigger compiler diagnostics
   - The compiler may pretty-print surrounding OpenMP constructs in error messages
   - Using `-fdump-tree-omp` will force pretty-printing of all OpenMP clauses
   - Higher optimization levels may trigger additional internal checks

**Compilation Commands for Coverage:**
