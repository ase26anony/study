This code appears to be part of a loop nesting analysis algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps:

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no common basic blocks**
   - If they don't intersect at all, `continue` (skip further processing)

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if A has any bits that are NOT in B
   - So `!bitmap_intersect_compl_p(A, B)` means "A is a subset of B" (all bits in A are also in B)
   - If true: `other` is a **sub-loop** of `loop` → add `other` to `loop->loops`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks if `loop` is a subset of `other`
   - If true: `loop` is a **sub-loop** of `other` → add `loop` to `other->loops`

## What it's doing

This code is building a **loop nesting tree** by determining parent-child relationships between loops:

- If `other` ⊆ `loop` → `other` is child of `loop`
- If `loop` ⊆ `other` → `loop` is child of `other`
- If neither is a subset of the other (they partially overlap but neither contains the other fully), no relationship is established

## Example

Consider loops with these basic blocks:
- Loop A: blocks {1, 2, 3, 4}
- Loop B: blocks {2, 3}

The code would detect that B ⊆ A, so B would be added as a child of A.

## Potential Issue

There's a missing case: what if the loops are **identical** (same block set)? Both conditions would be true, making them children of each other, which might create a cycle. This should probably be handled separately.
