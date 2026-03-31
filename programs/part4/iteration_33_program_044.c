This code appears to be part of a loop analysis or optimization pass in a compiler (likely GCC). Let me break down what it's doing:

## Code Analysis

The code is checking relationships between loops based on their block bitmaps:

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no common basic blocks**
   - If they don't intersect at all, `continue` (skip further processing)

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if `A & ~B` is empty (i.e., if all bits in A are also in B)
   - This means: **Is `other` completely contained within `loop`?**
   - If true: `loop->loops.safe_push(other)` - add `other` as a subloop of `loop`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks the opposite: **Is `loop` completely contained within `other`?**
   - If true: `other->loops.safe_push(loop)` - add `loop` as a subloop of `other`

## What it's doing

This code is **building a loop nesting hierarchy** by determining parent-child relationships between loops based on their basic block sets:

- If loop A contains all blocks of loop B, then B is a subloop of A
- If loop B contains all blocks of loop A, then A is a subloop of B
- If they partially overlap but neither fully contains the other, no relationship is established (this shouldn't happen in well-formed CFGs)

## Example

Consider:
- Loop X has blocks: {1, 2, 3, 4}
- Loop Y has blocks: {2, 3}

The code would detect that Y is completely contained within X (Y ∩ ~X = ∅), so Y becomes a subloop of X.

This is typical in compiler loop analysis where inner loops are nested within outer loops, and the compiler needs to build this hierarchy for optimizations like loop-invariant code motion, vectorization, etc.
