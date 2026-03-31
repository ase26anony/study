## Key Design Features:

1. **Partial Basic Block Overlap**: Each test function uses `if/else` conditions to create loops where inner loop basic blocks are subsets of outer loop blocks, but the outer loop has additional blocks not in the inner loop.

2. **Multiple Nesting Levels**: `test_three_level_nesting` creates three-level deep loops with varying containment relationships.

3. **Sibling Loops**: `test_sibling_loops` creates two inner loops that are siblings (both inside the outer loop but not nested within each other).

4. **Volatile Variables and Side Effects**: The `side_effect` function writes to a `volatile` array, preventing dead code elimination. The `checksum` variable accumulates results.

5. **Loop-Invariant but Non-Hoistable Code**: `test_with_invariants` uses `volatile` variables inside loops that appear invariant but can't be hoisted due to volatility.

6. **Varied Loop Structures**: Different loop directions, conditions, and bounds create diverse basic block patterns for the bitmap intersection analysis.

## Compilation Recommendations:

For RISC-V hardware loop coverage:
