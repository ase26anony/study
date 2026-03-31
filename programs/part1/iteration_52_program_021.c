This test program:

1. **Contains all required `depend` clause modifiers**: `in`, `inout`, `out`, `mutexinoutset`, `inoutset`, and `destroy` in valid OpenMP task constructs.

2. **Uses complex data environments**: 
   - Global, static, and extern variables
   - Class member variables (both static and non-static)
   - Local variables, static locals
   - Heap-allocated arrays
   - References and pointers
   - Array element accesses

3. **Includes nested and compound constructs**:
   - Tasks inside `parallel` regions
   - Tasks inside `single` constructs
   - `taskgroup` for structured task waiting
   - Combination with other clauses: `priority`, `mergeable`, `final`, `nowait`

4. **Creates realistic dependency patterns**:
   - Producer-consumer chains
   - Multiple dependencies per task
   - Set-based dependencies on array elements
   - Mixed dependency types in single tasks

5. **Triggers pretty-printing**:
   - The `trigger_diagnostics()` function contains a deliberate syntax error (commented out)
   - When compiled with `-fdump-tree-omp`, the compiler will generate dump files showing pretty-printed OpenMP constructs
   - Higher optimization levels may trigger additional internal verification
   - The `-Werror=openmp-format` flag can catch any format warnings

6. **Provides verifiable computation**:
   - The program performs actual computations
   - Outputs results to ensure code is semantically valid
   - Can be run to verify OpenMP execution

To maximize coverage of the uncovered lines, compile with:
