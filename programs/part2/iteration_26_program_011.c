This program combines all the required elements:

1. **Loop-carried dependencies**: Nested loops where inner loop count depends on outer index
2. **Mixed data types and non-contiguous access**: `MixedData` struct with different types and pointer arithmetic
3. **Inline assembly with clobbers**: Multiple `asm volatile` statements with different register clobbers
4. **Vector intrinsics**: SSE/AVX intrinsics for SIMD operations
5. **Function calls**: Library functions (`sin`, `cos`, `pow`) and custom helper functions
6. **Conditional moves**: Ternary operators used in hot loops
7. **Computed goto**: Jump table implementation
8. **Loop unrolling pragmas**: `#pragma GCC unroll` directives
9. **Switch-like control flow**: Implemented via computed goto
10. **Optimization attributes**: `__attribute__((optimize(...)))` on key functions

The program creates sustained computational pressure with varied instruction patterns, forcing the selective scheduler to analyze and potentially log many instructions through `dump_insn_rtx_1`. The final checksum ensures no dead code elimination.

Compile with:
