## Key Design Features:

1. **Partial Basic Block Overlap**: Each test function uses `if/else` conditions to create paths where inner loops execute only in specific branches, ensuring the inner loop's basic blocks are a proper subset of the outer loop's blocks.

2. **Multiple Nesting Levels**: `test_three_level_nesting` creates three levels of loops with varying containment relationships.

3. **Sibling Loops**: `test_sibling_loops` creates two inner loops at the same nesting level with different conditions, both partially overlapping with the outer loop.

4. **Loop-Invariant Code**: Uses `volatile` variables and calls to `side_effect()` to prevent optimization hoisting and ensure side effects.

5. **Complex Conditions**: Uses modulo operations and random values to create non-trivial control flow that the compiler can't easily simplify.

6. **Observable Side Effects**: All loops write to global `volatile` arrays and update a checksum to prevent dead code elimination.

## Compilation Recommendations:

For RISC-V hardware loop testing:
