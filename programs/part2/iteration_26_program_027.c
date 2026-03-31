This program combines all the required elements:

1. **Loop-carried dependencies**: Nested loops where inner loop count depends on outer index
2. **Mixed data types and non-contiguous access**: `MixedData` struct with different types accessed in patterns
3. **Inline assembly with clobbers**: Multiple asm blocks clobbering different registers
4. **Vector intrinsics**: SSE4.2 operations in `process_simd()`
5. **Function calls in hot paths**: Calls to `helper_func()`, `sin()`, `cos()`, `pow()`, etc.
6. **Conditional moves**: Ternary operators in performance-critical sections
7. **Computed goto**: Jump table implementation in `jump_table_example()`
8. **Loop unrolling pragmas**: Explicit unroll hints
9. **Switch statements**: Mixed dense/sparse cases in `process_switch()`
10. **Compiler attributes**: `__attribute__((optimize(...)))` to force specific optimizations

The program creates complex scheduling scenarios that should trigger the selective scheduler's debug output, particularly the `dump_insn_rtx_1` function with the uncovered switch to `stderr`.

Compile with:
