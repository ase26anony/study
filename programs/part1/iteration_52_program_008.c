**Explanation of Coverage Strategy:**

1. **All Depend Modifiers Covered:**
   - `depend(in: ...)` - lines 31, 73, 101
   - `depend(inout: ...)` - lines 39, 109
   - `depend(out: ...)` - lines 47, 115
   - `depend(mutexinoutset: ...)` - lines 54, 121, 156
   - `depend(inoutset: ...)` - lines 60, 127, 161
   - `depend(destroy: ...)` - lines 67, 133

2. **Complex Data Environment:**
   - Global variables (`global_var`)
   - Static variables (`static_global_var`, `static_local`)
   - Heap-allocated pointers (`heap_ptr`, `local_ptr`, `ptr_array`)
   - References (`ref`)
   - Array elements (`array[0]`, `local_ptr[2]`)
   - Class members (`member_var`, `static_member`)
   - Mixed storage durations and linkages

3. **Nested and Compound Constructs:**
   - Tasks inside `parallel` regions (lines 30, 38, etc.)
   - Tasks inside `single` constructs
   - `taskwait` directives (lines 70, 138, 152)
   - `taskgroup` (lines 141-150)
   - Combined with other clauses: `priority`, `mergeable`, `final`, `shared`

4. **Triggering Pretty-Printing:**
   - The deliberate syntax error `UndeclaredType error_var;` on line 189 will force the compiler's diagnostic machinery to engage
   - When using `-fdump-tree-omp` or `-fdump-tree-all`, the compiler will output pretty-printed OpenMP constructs to dump files
   - The `-Werror=openmp-format` option (if used) may trigger additional pretty-printing during format checking

**Compilation Commands for Coverage:**
