## Key Design Elements:

1. **Decrementing Loop Patterns**:
   - `for (volatile int i = outer_counter; i != 0; i--)` - Classic decrementing for loop
   - `while (temp-- != 0)` - Post-decrement in condition (targets PLUS with -1)
   - `while (n--)` - Implicit comparison to zero
   - `while (--k != 0)` - Pre-decrement in do-while condition

2. **Volatile Counters**: All loop counters are declared `volatile` to prevent the compiler from optimizing away the specific decrement-and-compare pattern during early passes.

3. **Nested Structures**: Multiple levels of nesting with independent `volatile` counters to increase matching opportunities.

4. **Side Effects**: 
   - `side_effect()` function with empty `asm volatile` to prevent loop removal
   - Modifications to `global_result` and `static_sum` create data dependencies

5. **Mixed Constructs**: Combination of `for`, `while`, and `do-while` loops to test pattern recognition across different high-level constructs.

## Compilation Recommendations:
