This test program:

1. **Covers all depend modifiers**: `in`, `inout`, `out`, `mutexinoutset`, `inoutset`, and `destroy` are all used in valid OpenMP task constructs.

2. **Uses complex data environment**: 
   - Global variables (`global_var`, `static_global_var`)
   - Static variables (`static_member`, `static_local`)
   - Heap-allocated pointers (`heap_ptr`, `local_ptr`)
   - References (`ref`)
   - Array elements (`local_array[2]`, `heap_ptr[0]`)
   - Class member variables (`member_var`)
   - Pointer dereferences (`*ptr`)

3. **Includes nested and compound constructs**:
   - Tasks inside `parallel` regions
   - Tasks inside `single` constructs
   - `taskgroup` for synchronization
   - `taskwait` for dependency resolution
   - Combined with other clauses like `priority`, `final`, `mergeable`, `nowait`

4. **Triggers pretty-printing execution**:
   - The `trigger_diagnostics()` function contains a deliberate syntax error (`UndeclaredType`) that will cause the compiler to emit an error. When this happens, the compiler's diagnostic machinery may pretty-print the surrounding OpenMP constructs.
   - Compiling with `-fdump-tree-omp` or `-fdump-tree-all` will generate dump files that invoke the pretty-printer.
   - The `-Werror=openmp-format` option may also trigger pretty-printing during format checking.

5. **Has valid execution flow**:
   - The `main()` function performs actual computations with dependencies
   - Results are printed to verify correct execution
   - The code can be compiled and run (if the `trigger_diagnostics()` call is commented out)

To maximize coverage of the pretty-printer lines, compile with:
