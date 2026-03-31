## Key Design Elements:

1. **Decrementing Loop Counter with Compare-to-Zero**: All loops use `i != 0`, `count-- != 0`, or similar zero comparisons.

2. **Post-Decrement in Condition**: Patterns like `while (temp-- != 0)` and `for (; n--; )` ensure the decrement happens as part of the comparison expression, which should generate the `(PLUS reg -1)` RTL pattern.

3. **Volatile Loop Bounds**: All loop counters are declared `volatile` to prevent early optimizations from altering the decrement-and-compare pattern.

4. **Nested Loops**: Multiple levels of nesting with independent `volatile` counters increase matching opportunities.

5. **Mixed Loop Constructs**: Combines `for`, `while`, and `do-while` loops with both pre-decrement (`--k != 0`) and post-decrement (`temp-- != 0`) patterns.

6. **Side Effects**: The `side_effect()` function (using inline asm) and assignments to `global_result` prevent dead code elimination without obscuring the target RTL pattern.

## Compilation Recommendations:
