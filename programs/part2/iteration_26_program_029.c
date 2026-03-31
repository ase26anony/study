This program combines all the required patterns:

1. **Loop-carried dependencies**: Nested loops where inner loop count depends on outer index
2. **Mixed data types**: `MixedData` struct with int, double, char, short, float
3. **Inline assembly with clobbers**: Multiple `asm volatile` statements with different clobbered registers
4. **Vector intrinsics**: SSE/AVX intrinsics (`_mm_loadu_si128`, `_mm_add_epi32`, etc.)
5. **Function calls**: Library functions (`sin`, `pow`, `sinf`, `cosf`) and custom helpers
6. **Conditional moves**: Ternary operators in hot loops
7. **Computed goto**: Jump table implementation
8. **Loop unrolling pragmas**: `#pragma GCC unroll`
9. **Switch statements**: Mix of dense and sparse cases
10. **Optimization attributes**: `__attribute__((optimize(...)))` on key functions

**Compilation command to trigger the uncovered code:**
