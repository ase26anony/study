## Key Features That Trigger Selective Scheduler Debug Output:

1. **Complex Loop Structures**: Nested loops with data-dependent trip counts create challenging scheduling scenarios.

2. **Mixed Operations**: Integer, floating-point, and SIMD operations in the same basic blocks force the scheduler to handle diverse instruction types.

3. **Inline Assembly with Clobbers**: Multiple `asm volatile` statements with register clobbers create hard constraints that the scheduler must work around.

4. **Memory Access Patterns**: Non-contiguous and pointer-based access patterns generate varied load/store instructions.

5. **Control Flow Diversity**: Computed gotos, switch statements, and conditional operators create complex control flow graphs.

6. **Compiler Pragmas**: `#pragma GCC unroll` creates large basic blocks that challenge the scheduler.

7. **Function Attributes**: `__attribute__((optimize(...)))` ensures specific optimization flags are applied to key functions.

## Compilation and Execution:
