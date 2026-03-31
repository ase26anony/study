This program comprehensively tests the uncovered partition code mapping by:

1. **Exhaustive Enumeration**: Tests partition codes from -1 to 9, covering all valid codes (0-7) and illegal values.

2. **Template Metaprogramming**: Generates specialized OpenACC code for each partition code at compile time through template specializations.

3. **Volatile Control Flow**: Uses `volatile int` variables to prevent compiler optimization and constant folding, ensuring all code paths remain in the binary.

4. **Complex Partitioning Patterns**: 
   - Gang redundant clauses
   - Individual gang/worker/vector partitioning
   - Combined partitioning (gang+worker, gang+vector, worker+vector)
   - Fully partitioned 3D loops
   - Mixed partitioning for illegal codes

5. **Multiple OpenACC Constructs**: Uses both `parallel` and `kernels` constructs with various data clauses.

6. **Multi-dimensional Arrays**: Uses 2D and simulated 3D array access patterns.

7. **Prevents Dead Code Elimination**: Includes final reductions and output to ensure all computations are preserved.

**Compilation commands for coverage analysis:**
