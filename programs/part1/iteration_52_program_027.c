**Key Features of This Test Program:**

1. **All `depend` Modifiers Covered:**
   - `depend(in: ...)` - Lines 32-33, 67-68, 96-97
   - `depend(inout: ...)` - Lines 38-39, 71-72, 100-101
   - `depend(out: ...)` - Lines 44-45, 64, 91-92, 122-123
   - `depend(mutexinoutset: ...)` - Lines 50-51, 73, 103-104, 127-128
   - `depend(inoutset: ...)` - Lines 56, 74, 105, 129
   - `depend(destroy: ...)` - Lines 60-61, 75, 106, 130

2. **Complex Data Environment:**
   - Global variables (`global_var`)
   - Static variables (`static_global_var`, `static_local`)
   - Heap-allocated pointers (`heap_ptr`, `local_heap`, `double_ptr`)
   - References (`ref`)
   - Array elements (`array[0]`, `heap_ptr[0]`)
   - Class member variables (`member_var`, `static_member`)
   - Pointer dereferencing (`*ptr`)

3. **Nested and Compound Constructs:**
   - Tasks inside `parallel` regions
   - `taskgroup` for structured task dependencies
   - `taskwait` for synchronization
   - Combined with other clauses: `priority`, `final`, `mergeable`

4. **Trigger for Pretty-Printing:**
   - The deliberate syntax error `UndeclaredType error_var;` on line 149 will force the compiler to emit an error message
   - When compiled with `-fdump-tree-omp`, the OpenMP constructs will be dumped
   - With `-Werror=openmp-format`, any OpenMP format warnings become errors
   - The `-fdump-tree-all` option will generate multiple dump files showing pretty-printed clauses

5. **Valid Execution Flow:**
   - The program performs actual computations
   - Uses reduction for verification
   - Outputs a result to ensure OpenMP directives are active
   - Proper memory management

**Compilation Commands to Test Coverage:**
