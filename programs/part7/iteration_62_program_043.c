## Key Design Elements:

1. **Decrementing Loop Patterns**: Each test function implements a different variant of the decrementing loop pattern that should generate the RTL `(compare (plus reg -1) (const_int 0))`.

2. **Register Pressure**: Each function uses multiple `volatile` variables and temporary computations to create register pressure, forcing the loop counter to remain in a register.

3. **Control Flow Inside Loops**: Each loop contains `if` statements, `switch` statements, conditional `continue`, or early exit conditions to create non-trivial loop bodies.

4. **Post-Loop Counter Use**: Each function returns a value that incorporates the loop counter or its final state, preventing dead code elimination.

5. **No Inlining**: All test functions are marked `noinline` to prevent interprocedural optimizations from altering the loop structure.

6. **Multiple Variants**: The program includes:
   - `for` loop with `i != 0` condition
   - `while` loop with post-decrement
   - `do-while` loop with pre-decrement
   - Manual decrement inside loop body
   - Mixed type operations

## Compilation Recommendations:
