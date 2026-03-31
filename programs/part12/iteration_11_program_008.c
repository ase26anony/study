## Key Design Elements:

1. **Partial Basic Block Overlap**: Each test function creates loops where inner loops share some but not all basic blocks with their outer loops through:
   - Conditional statements (`if`, `switch`) that place inner loops in specific branches
   - Code before/after inner loops within the outer loop body
   - Multiple sibling inner loops that execute under different conditions

2. **Multiple Nesting Levels**: The code includes up to 3 levels of nesting (e.g., `i` → `j` → `k` loops) with varying containment relationships.

3. **Side Effects and Volatility**: 
   - `volatile int results[1000]` prevents dead code elimination
   - `process_value()` function with global side effects
   - `rand()` calls create loop-variant conditions

4. **Countable Loops**: All loops have compile-time or runtime-determinable bounds suitable for hardware loop analysis.

5. **Diverse Patterns**: 
   - Simple conditional nesting (`nested_loops_with_partial_overlap`)
   - Complex multi-level nesting (`complex_nesting_pattern`)
   - Switch-based control flow (`switch_based_nesting`)
   - Additional varying-bound loops in `main()`

## Compilation Recommendations:

For RISC-V hardware loop testing:
