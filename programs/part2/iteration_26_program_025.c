This program combines all the required elements:

1. **Loop-carried dependencies**: Nested loops where inner loop count depends on outer index
2. **Mixed data types and non-contiguous access**: `MixedData` struct with different types and sizes
3. **Inline assembly with clobbers**: Multiple `asm volatile` statements with different register clobbers
4. **Vector intrinsics**: SSE/AVX operations using `_mm_loadu_ps`, `_mm_mul_ps`, etc.
5. **Function calls in hot paths**: `compute_helper` called within loops
6. **Conditional moves**: Ternary operators in performance-critical sections
7. **Computed goto**: Jump table implementation for complex control flow
8. **Loop unrolling pragmas**: `#pragma GCC unroll` directives
9. **Switch statements**: Both dense and sparse cases
10. **Compiler attributes**: `__attribute__((optimize(...)))` to force specific optimizations
11. **Matrix operations**: Complex access patterns that challenge the scheduler

To compile and run for coverage:
