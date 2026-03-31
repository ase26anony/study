This program creates the necessary conditions to trigger the uncovered selective scheduler debug output:

1. **Complex Loop Structures**: Nested loops with data-dependent trip counts create challenging scheduling scenarios.

2. **Mixed Operations**: Combines integer, floating-point, SIMD, and conditional operations to generate diverse RTL patterns.

3. **Inline Assembly**: Multiple `asm volatile` statements with different clobber lists force the scheduler to work around fixed constraints.

4. **Control Flow Diversity**: Uses computed goto, switch statements, and conditional moves to create complex control flow graphs.

5. **Memory Access Patterns**: Non-contiguous access through structure members and array indexing creates varied memory dependencies.

6. **Compiler Directives**: Uses `__attribute__((optimize))` with selective scheduling flags and `#pragma GCC unroll` to influence scheduling decisions.

7. **Function Calls**: Calls to math functions (`sin`, `cos`, `pow`) and custom helper functions create call instructions with varying register usage.

To maximize coverage, compile with:
