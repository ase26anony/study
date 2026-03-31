## Key Design Elements:

1. **Partially Overlapping Loop Blocks**: The two inner loops in `generate_nested_loops` share basic blocks through:
   - Identical `if ((i + j) % 3 == 0)` blocks
   - Shared `switch` case handlers (cases 2 and 3)
   - This creates bitmap intersection without full containment

2. **Complex Control Flow**:
   - `goto` early exit from first inner loop to outer scope
   - `switch` statements with fall-through
   - Mixed `break`, `continue`, and `goto` statements

3. **Loop Transformations**:
   - Manual 4x unrolling in first inner loop
   - `#pragma GCC unroll 2` in second inner loop
   - Mixed computation pattern encouraging loop distribution

4. **Data-Dependent Bounds**:
   - Loop bounds using `rand() % N + constant`
   - Volatile variables affecting control flow
   - Early exits based on array values

5. **Multiple Loop Hierarchies**:
   - Recursive generation creates loops at different depths
   - Separate `overlapping_loop_pattern` function creates different overlap pattern
   - Multiple calls with varying parameters in `main()`

## Compilation Recommendations:
