## Key Design Elements:

1. **Decrementing Loop Patterns**: Each test function implements a different variant of decrementing loops that should generate the `(compare (plus reg -1) (const_int 0))` RTL pattern.

2. **Register Pressure**: Each function uses numerous `volatile` variables that force the compiler to keep values in registers, preventing the loop counter from being spilled to memory.

3. **Control Flow Inside Loops**: All loops contain `if` statements, `switch` cases, or conditional `break`/`continue` to create non-trivial loop bodies.

4. **Post-Loop Counter Use**: Each function returns a value that incorporates the final counter state, preventing dead code elimination.

5. **No Inlining**: `__attribute__((noinline))` ensures each loop remains in its own function for the doloop pass to analyze independently.

6. **Moderate Iteration Counts**: Loop bounds are between 20-100 iterations, avoiding extremes that might trigger different optimizations.

## Compilation Recommendations:
