This program combines all the required elements:

1. **Loop-carried dependencies**: `nested_loop_computation` has inner loops with trip counts dependent on outer loop indices.

2. **Mixed data types and non-contiguous access**: `MixedData` struct with different types and non-sequential access patterns.

3. **Inline assembly with clobbers**: SIMD function includes inline assembly that clobbers xmm0 and memory.

4. **Vector intrinsics**: SSE/AVX intrinsics for SIMD operations.

5. **Function calls in loops**: Calls to `compute_helper`, `sin`, `cos`, `pow`, `fabs`.

6. **Conditional moves**: Ternary operators in tight loops.

7. **Computed goto**: `computed_goto_test` uses labels as values for indirect branching.

8. **Loop unrolling pragmas**: `#pragma GCC unroll` directives.

9. **Switch with dense/sparse cases**: `dense_sparse_switch` tests different jump table implementations.

10. **Optimization attributes**: Functions marked with `__attribute__((optimize(...)))` to ensure selective scheduling is enabled.

Compile with:
