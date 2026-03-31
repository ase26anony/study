This program combines all the required elements:

1. **Loop-carried dependencies**: Nested loops where inner loop trip count depends on outer loop index
2. **Mixed data types**: `MixedData` struct with int, double, char array, and float
3. **Inline assembly with clobbers**: Multiple `asm volatile` statements clobbering registers
4. **Vector intrinsics**: AVX and SSE instructions for SIMD processing
5. **Function calls in loops**: Calls to `sin()`, `powf()` within loops
6. **Conditional moves**: Ternary operators used in performance-critical sections
7. **Computed goto**: Jump table implementation for indirect branching
8. **Loop unrolling pragmas**: `#pragma GCC unroll` directives
9. **Complex switch statements**: Mix of dense and sparse cases
10. **Optimization attributes**: `__attribute__((optimize(...)))` on key functions

**Compilation recommendations:**

1. **For maximum scheduler activation**:
