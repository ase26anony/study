**Explanation of Coverage Strategy:**

1. **All Depend Modifiers Covered:**
   - `depend(in: global_var)` - line 47
   - `depend(inout: static_global_var)` - line 53
   - `depend(out: member_var)` - line 59
   - `depend(mutexinoutset: global_array[2])` - line 65
   - `depend(inoutset: global_array[3])` - line 71
   - `depend(destroy: *heap_ptr)` - line 77
   - Additional `destroy` clause on line 103

2. **Complex Data Environment:**
   - Global variables (`global_var`)
   - Static global variables (`static_global_var`)
   - Heap-allocated pointers (`heap_ptr`, `local_heap`)
   - Array elements (`global_array[2]`, `global_array[3]`)
   - References (`ref`) and dereferenced pointers (`*ptr`)
   - Class member variables (`member_var`)
   - Struct members (`point.x`, `point.y`)

3. **Nested and Compound Constructs:**
   - Tasks inside `parallel` region (line 44)
   - Tasks inside `single` construct (line 45)
   - Tasks inside `taskgroup` (lines 89-98)
   - `taskwait` for synchronization (line 101)
   - Combined with other clauses: `priority`, `mergeable`, `final`

4. **Triggering Pretty-Printing:**
   - The deliberate syntax error on line 122 (`UndeclaredType x`) will force the compiler's diagnostic machinery to engage
   - When compiling with `-fdump-tree-omp`, the OpenMP constructs will be dumped in pretty-printed form
   - The `-Werror=openmp-format` option (if used) may also trigger pretty-printing during format checking

**Compilation Commands for Coverage:**
