This program combines all the required elements:

1. **Loop-carried dependencies**: `nested_loop_computation` has inner loop bounds dependent on outer loop index
2. **Mixed data types and non-contiguous access**: `MixedData` struct with different types and SIMD operations
3. **Inline assembly with clobbers**: Multiple `asm volatile` statements clobbering specific registers
4. **Vector intrinsics**: SSE/AVX intrinsics (`_mm_loadu_ps`, `_mm_add_epi32`, etc.)
5. **Function calls with varying arguments**: Calls to `helper_func`, `sin`, `cos`, `pow`
6. **Conditional moves**: Ternary operators in hot loops
7. **Computed goto**: Jump table implementation in `jump_table_computation`
8. **Loop unrolling pragmas**: `#pragma GCC unroll` directives
9. **Sparse switch cases**: `sparse_switch` function with widely spaced case values
10. **Optimization attributes**: `__attribute__((optimize(...)))` to force specific optimization flags

The program creates complex scheduling scenarios that should trigger the selective scheduler's debug output, including calls to `dump_insn_rtx_1` for many different instruction types.

To compile and run for maximum coverage:
