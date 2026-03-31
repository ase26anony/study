Looking at this code, it appears to be handling relationships between loops in a control flow graph. Let me break down what this code is doing:

## Code Analysis

The code checks relationships between two loops (`loop` and `other`) based on their block bitmaps:

1. **First check**: `bitmap_intersect_p(other->block_bitmap, loop->block_bitmap)`
   - This checks if the two loops share any basic blocks
   - If they don't share any blocks (`!bitmap_intersect_p` returns true), `continue` skips further processing

2. **Second check**: `bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)`
   - This checks if `other->block_bitmap` has any bits set that are NOT set in `loop->block_bitmap`
   - If `other` has NO blocks outside of `loop` (the `!` negates the result), then `other` is completely contained within `loop`
   - In this case, `other` is added as a child/subloop of `loop`

3. **Third check**: `bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)`
   - This checks if `loop->block_bitmap` has any bits set that are NOT set in `other->block_bitmap`
   - If `loop` has NO blocks outside of `other` (the `!` negates the result), then `loop` is completely contained within `other`
   - In this case, `loop` is added as a child/subloop of `other`

## What This Code Does

This code is **building a loop hierarchy** (nesting structure) by determining parent-child relationships between loops:

- If loop A contains loop B (all of B's blocks are within A), then B becomes a child of A
- If loop B contains loop A (all of A's blocks are within B), then A becomes a child of B
- If loops partially overlap (share some blocks but each has blocks the other doesn't), neither becomes a child of the other

## Example

Consider three loops with these block sets:
- Loop A: blocks {1, 2, 3, 4}
- Loop B: blocks {2, 3}
- Loop C: blocks {3, 4, 5}

Results:
- A vs B: B is completely within A → B becomes child of A
- A vs C: They share blocks 3 and 4, but C has block 5 not in A, and A has blocks 1,2 not in C → no parent-child relationship
- B vs C: They share block 3, but B has block 2 not in C, and C has blocks 4,5 not in B → no parent-child relationship

This appears to be from a compiler's loop analysis phase, likely in GCC or a similar compiler infrastructure.
