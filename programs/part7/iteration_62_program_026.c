## Key Design Elements for Triggering the Target Code:

1. **Decrementing Loop Patterns**: Each test function implements a different variant of decrementing loops that should generate the RTL pattern `(compare (plus reg -1) (const_int 0))`:
   - `for (int i = n; i != 0; i--)` - exact pattern match
   - `while (counter-- > 0)` - post-decrement with comparison
   - `do { ... } while (--count)` - pre-decrement in condition
   - Manual `i -= 1` with explicit `if (i == 0) break`

2. **Register Pressure**: Each function uses multiple `volatile` variables and many temporary variables to force register allocation pressure, encouraging the counter to stay in a register.

3. **Control Flow in Loop Bodies**: Each loop contains `if` statements and conditional logic to create non-trivial loop bodies that the doloop pass must analyze.

4. **Post-Loop Counter Use**: Each function uses the counter value after the loop ends to prevent dead code elimination.

5. **Moderate Iteration Counts**: Loop counts between 17-50 iterations avoid unrolling while providing enough work for optimization.

6. **Mixed Signed/Unsigned**: Uses both signed and unsigned counters to test different comparison patterns.

## Compilation Recommendations:
