This code appears to be part of a loop nesting analysis or control flow graph processing algorithm. Let me break down what it's doing:

## Code Analysis

The code processes relationships between loops based on their block bitmaps (sets of basic blocks):

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if `other` loop and `loop` have **no blocks in common**
   - If they don't share any blocks, `continue` (skip processing this pair)

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if `A` has any bits set that are **not** in `B`
   - So this condition is true when **all blocks of `other` are contained within `loop`**
   - If true: `loop->loops.safe_push(other)` - adds `other` as a nested loop inside `loop`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - This checks if **all blocks of `loop` are contained within `other`**
   - If true: `other->loops.safe_push(loop)` - adds `loop` as a nested loop inside `other`

## What it's determining

This code is building a **loop nesting tree** by comparing which loops are properly nested within others:

- If loops share no blocks → unrelated (skipped)
- If `other` is completely inside `loop` → `other` is nested in `loop`
- If `loop` is completely inside `other` → `loop` is nested in `other`
- If neither is completely inside the other → they overlap but neither contains the other (no action)

## Example

Consider three loops with these blocks:
- Loop A: blocks {1, 2, 3, 4}
- Loop B: blocks {2, 3}
- Loop C: blocks {3, 4, 5}

Processing A and B:
- They intersect (share blocks 2, 3)
- All of B's blocks are in A → B nested in A

Processing A and C:
- They intersect (share block 3, 4)
- Neither is completely inside the other → no nesting relationship

This is typical compiler code for building a loop hierarchy in control flow analysis.
