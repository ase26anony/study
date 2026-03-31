## Key Features of This Test Program:

1. **Complete Coverage of All `depend` Modifiers**:
   - `depend(in: ...)` - lines 31, 37, 45, 61, 72, 89, 103, 110
   - `depend(inout: ...)` - lines 38, 62, 73, 90, 112
   - `depend(out: ...)` - lines 44, 71, 103, 110
   - `depend(mutexinoutset: ...)` - lines 49, 50, 91, 111
   - `depend(inoutset: ...)` - lines 55, 56, 92, 93, 112
   - `depend(destroy: ...)` - lines 60, 97, 113

2. **Varied Variable Contexts**:
   - Global variables (`global_var`, `static_global_var`)
   - Static variables (`static_local`)
   - Heap-allocated pointers (`heap_ptr`, `member_ptr`)
   - Array elements (`global_array[0]`, `ptr_arr[0][0]`)
   - References (`local_ref`)
   - Class member variables (`member_var`)
   - Local variables with different scopes

3. **Complex OpenMP Structures**:
   - Tasks inside `parallel` regions
   - `taskgroup` for nested task dependencies
   - `taskwait` for synchronization
   - Combination with other clauses (`priority`, `final`, `mergeable`, `nowait`)

4. **Trigger for Pretty-Printing**:
   - The `trigger_diagnostic()` function contains a deliberate syntax error (`UndeclaredType`)
   - When compiled, this will force the compiler's diagnostic machinery to engage
   - The error is in a separate function to avoid affecting the main compilation flow

## Recommended Compilation Commands:

1. **For Tree Dumping (Primary Method)**:
