## Key Design Elements:

1. **Decrementing Loop Patterns**: Each loop uses counters that decrement by 1 and compare to zero:
   - `for (volatile int i = counter1; i != 0; i--)`
   - `while (j-- != 0)`
   - `for (; n--; )`
   - `while (--m != 0)`

2. **Volatile Counters**: All loop counters are declared `volatile` to prevent the compiler from optimizing away the specific decrement-and-compare pattern during early optimization passes.

3. **Post-Decrement in Conditions**: Multiple loops use post-decrement operators (`j--`, `n--`) directly in the conditional expression, which should generate the `PLUS` with `-1` RTL pattern.

4. **Nested Loops**: The program includes multiple levels of nested loops where both inner and outer loops use decrementing patterns with independent counters.

5. **Mixed Constructs**: Combines `for`, `while`, and `do-while` loops with different decrement patterns to test the pass across various high-level constructs.

6. **Side Effects**: Each loop body calls `side_effect()` which uses inline assembly to create a non-removable side effect, preventing the loops from being eliminated as dead code.

## Compilation Recommendations:

To specifically target the loop-doloop pass and see the RTL transformations:
