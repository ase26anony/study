## Key Design Elements:

1. **Partial Basic Block Overlap**: Each test function creates loops where inner loops share some but not all basic blocks with outer loops through conditional execution (`if` statements).

2. **Multiple Nesting Levels**: 
   - `test_partial_overlap_nesting()`: 2-3 levels with varying conditions
   - `test_three_level_nesting()`: Explicit 3-level nesting with partial overlap
   - `test_sibling_loops()`: Multiple inner loops at the same level (siblings)

3. **Complex Control Flow**:
   - Early exits with `break`
   - Multiple conditional branches within loops
   - Different loop bounds based on outer loop indices

4. **Side Effects Prevention**:
   - `volatile` array writes
   - `__attribute__((noinline))` function
   - Global variable modifications
   - Final output to prevent dead code elimination

5. **Loop Characteristics**:
   - Countable loops with compile-time or runtime-known bounds
   - Simple induction variables
   - Mixed loop patterns to trigger different bitmap intersection scenarios

## Compilation Recommendations:

For RISC-V hardware loop testing:
