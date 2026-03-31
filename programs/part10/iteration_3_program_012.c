Looking at this code, it appears to be handling relationships between loops in a control flow graph. Let me break down what this code is doing:

## Code Analysis

This code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps and building a hierarchy:

1. **First check**: `bitmap_intersect_p(other->block_bitmap, loop->block_bitmap)`
   - Checks if the two loops share any basic blocks
   - If they don't share any blocks (`!bitmap_intersect_p`), skip to next iteration with `continue`

2. **Second check**: `bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)`
   - Checks if `other` has any blocks that are NOT in `loop`
   - If `other` has NO blocks outside `loop` (meaning all of `other`'s blocks are within `loop`), then `other` is nested inside `loop`
   - In this case: `loop->loops.safe_push(other)` - add `other` as a child of `loop`

3. **Third check**: `bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)`
   - Checks if `loop` has any blocks that are NOT in `other`
   - If `loop` has NO blocks outside `other` (meaning all of `loop`'s blocks are within `other`), then `loop` is nested inside `other`
   - In this case: `other->loops.safe_push(loop)` - add `loop` as a child of `other`

## What This Code Does

This code is building a loop nesting hierarchy. It determines parent-child relationships between loops based on their block containment:

- If loop A completely contains loop B (all of B's blocks are in A), then B becomes a child of A
- If loop B completely contains loop A (all of A's blocks are in B), then A becomes a child of B

## Important Note

The logic assumes that loops are either:
1. Disjoint (no shared blocks)
2. Nested (one completely contains the other)
3. Overlapping but not properly nested (this case isn't handled here)

The code doesn't handle the case where loops overlap but neither completely contains the other - in that case, neither condition would be true and no relationship would be established.

This is typical for natural loop analysis in compilers where loops are expected to be properly nested.
