This program comprehensively exercises the partition code mapping function through:

1. **Exhaustive Runtime Enumeration**: Tests partition codes -1 through 9 in nested loops with multi-dimensional arrays, covering all valid codes (0-7) and illegal values.

2. **Template Metaprogramming**: Generates specialized OpenACC code for each partition code at compile time via template instantiations 0-7 and illegal values.

3. **Mixed Partitioning Patterns**: Combines different partition clauses (gang redundant, gang partitioned, worker partitioned, etc.) in complex nested loop structures.

4. **Volatile Variables**: Uses `volatile int` to prevent constant folding and ensure all code paths are generated.

5. **Multi-dimensional Arrays**: Uses 3D arrays with broadcasting operations across different dimensions.

6. **Both Parallel and Kernels Constructs**: Tests both `#pragma acc parallel` and `#pragma acc kernels` regions.

7. **Prevention of Dead Code Elimination**: Includes final reductions and output to ensure all computations are preserved.

Compile with:
