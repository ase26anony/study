This program creates the following loop structures to exercise the bitmap intersection logic:

1. **Perfectly nested loops** (lines 124-141): Inner loop blocks are proper subsets of outer loop blocks, triggering `!bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)`.

2. **Partially overlapping loops** (via `overlapping_loops_test`): Loops share some blocks but each has unique blocks, causing both intersection checks to pass but complement checks to fail.

3. **Sibling loops** (via `sibling_loops_test`): Loops at the same level sharing no blocks, causing `bitmap_intersect_p` to return false.

4. **Inlined function loops**: Functions marked with `always_inline` create loop structures that get merged with caller's CFG.

5. **Loops with multiple entry/exit points**: Using `goto`, `break`, and `continue` statements to create non-contiguous basic block ranges.

6. **Mixed loop types**: `for`, `while`, `do-while`, and infinite loops with complex conditions.

7. **Recursive structures**: Creating loop-like patterns through recursion.

To compile for maximum coverage analysis:
