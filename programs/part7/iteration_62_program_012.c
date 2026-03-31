## Key Design Elements:

1. **Decrementing Loop Patterns**: Each test function implements a different variant of decrementing loops that should generate the `(compare (plus reg -1) (const_int 0))` RTL pattern.

2. **Register Pressure**: Each function creates 9-12 temporary variables that compete for registers, forcing the loop counter to remain in a register.

3. **Control Flow**: Each loop contains `if` statements, `switch`, `continue`, or `break` to create non-trivial loop bodies.

4. **Post-Loop Counter Use**: All functions use the counter value after the loop to prevent elimination.

5. **Volatile Variables**: Used to prevent optimizations that might eliminate register usage.

6. **Moderate Iteration Counts**: 25-100 iterations to avoid unrolling heuristics.

## Compilation Commands:
