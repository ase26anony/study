## Key Design Elements for Triggering the Target Code:

1. **Decrementing Loop Patterns**: Each test function implements a different variant of decrementing loops that should generate `(compare (plus reg -1) (const_int 0))`:
   - `for (int i = n; i != 0; i--)` - Classic decrementing for loop
   - `while (count-- > 0)` - Post-decrement while loop  
   - `while (--counter)` - Pre-decrement do-while condition
   - `while (i != 0) { i -= 1; }` - Decrement inside body

2. **Register Pressure**: Each function uses multiple integer variables and `volatile` operations to force the compiler to keep the loop counter in a register rather than spilling it to memory.

3. **Control Flow Inside Loops**: Each loop contains `if` statements, `switch`, `continue`, or `break` to create non-trivial loop bodies that the doloop pass must analyze.

4. **Post-Loop Counter Use**: Each function uses the loop counter's value after the loop ends (either directly or through related computations) to prevent dead code elimination.

5. **Moderate Iteration Counts**: Using 25-100 iterations avoids loop unrolling while keeping the loop substantial enough for optimization.

## Compilation Recommendations:
