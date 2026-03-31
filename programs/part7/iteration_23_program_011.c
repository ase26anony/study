## Key Design Elements:

1. **High Register Pressure**: 12+ volatile local variables that are used across function calls and inline assembly.

2. **Multiple Call Sites**: Three different helper functions and one external function call, each with different numbers of arguments to use different calling conventions.

3. **Strategic Inline Assembly**: Two `asm volatile` statements with explicit clobber lists for x86-64 caller-saved registers (both integer and SSE registers).

4. **Block Structure**: The inline assembly and function calls are placed:
   - Between variable uses (not at block boundaries)
   - With computations before and after each call
   - The `external_func` call is placed near the end of the loop body to potentially make it the `BB_END`

5. **Anti-Optimization Measures**:
   - `volatile` variables prevent dead store elimination
   - `__attribute__((noinline, noipa))` prevents function inlining
   - Complex computations keep variables live
   - Loop with 100 iterations provides repeated patterns
   - Final `printf` ensures side effects

## Compilation and Testing:
