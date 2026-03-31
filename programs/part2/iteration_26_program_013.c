## Key Features That Trigger Selective Scheduler Debugging:

1. **Loop-Carried Dependencies**: The `nested_loops_dependency` function has inner loops whose trip counts depend on outer loop indices, creating complex dependency chains.

2. **Mixed Data Types and Memory Access**: The `MixedData` structure and its processing create varied memory access patterns requiring different load/store operations.

3. **Inline Assembly with Clobbers**: Multiple `asm volatile` statements with different register clobbers force the scheduler to work around fixed constraints.

4. **SIMD Intrinsics**: SSE/AVX intrinsics generate specific RTL patterns that the selective scheduler must handle.

5. **Computed Goto**: The `computed_goto_switch` function uses labels-as-values for indirect jumps, challenging the scheduler's branch prediction.

6. **Loop Unrolling Pragmas**: `#pragma GCC unroll` directives create larger basic blocks for the scheduler to analyze.

7. **Function Attributes**: `__attribute__((optimize(...)))` ensures specific optimization flags are applied to key functions.

8. **Conditional Moves**: Ternary operators in hot loops encourage conditional move instruction generation.

## Compilation Instructions:
