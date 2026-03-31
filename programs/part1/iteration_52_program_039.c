**Key features of this test program:**

1. **All `depend` modifiers covered:**
   - `depend(in: ...)` - Lines 48, 81, 108, 140, 177
   - `depend(inout: ...)` - Lines 55, 112, 142, 177
   - `depend(out: ...)` - Lines 63, 115, 177
   - `depend(mutexinoutset: ...)` - Lines 70, 118, 154, 177
   - `depend(inoutset: ...)` - Lines 76, 121, 155, 177
   - `depend(destroy: ...)` - Lines 83, 124, 177

2. **Complex data environment:**
   - Global variables (`global_var`)
   - Static variables (`static_global`, `static_local`)
   - Heap-allocated pointers (`heap_array`, `local_ptr`)
   - Array elements (`heap_array[0]`, `local_array[0]`)
   - Class member variables (`member_var`)
   - Static class members (`static_member`)
   - References (`ref1`, `ref2` in `process_with_references`)

3. **Varied scoping contexts:**
   - Global scope
   - Class member functions
   - Static member functions
   - Template functions
   - Functions with reference parameters

4. **Nested and compound constructs:**
   - Tasks inside `parallel` regions
   - `taskgroup` for dependency management
   - `taskwait` for synchronization
   - Combined with other clauses: `priority`, `mergeable`, `final`, `if`

5. **Trigger for pretty-printing:**
   - The `function_with_error()` contains an undeclared type `UndeclaredType`
   - This will cause a compilation error, potentially triggering the compiler's diagnostic machinery to pretty-print the surrounding OpenMP constructs
   - The error is in a separate function so it doesn't affect the valid test code

**Compilation commands to trigger coverage:**
