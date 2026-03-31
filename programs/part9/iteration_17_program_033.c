## Key Features Targeting the Uncovered Logic:

1. **Perfect Nesting**: The `#pragma GCC unroll` directive creates perfectly nested loops where inner blocks are subsets of outer blocks.

2. **Partial Overlap**: The `do-while` loop inside `nested_loops_test` partially overlaps with the outer `for` loop, sharing some blocks but not all.

3. **Sibling Loops**: The `while` loop at the end of `nested_loops_test` and the loops in Test 4 are siblings at the same nesting level.

4. **Non-Contiguous Blocks**: Multiple `goto` statements (`early_exit`, `adjust_value`, `finish_chunk`) create jumps that break contiguous block ranges.

5. **Bitmap Intersection Scenarios**:
   - `bitmap_intersect_p` true, `bitmap_intersect_compl_p` false: Perfect nesting
   - Both true: Partial overlap
   - First true, second true for other direction: Reverse containment

6. **Complex Control Flow**: Switch statements with fall-through, multiple if-else chains, and conditional breaks create divergent basic blocks.

## Recommended Compilation:
