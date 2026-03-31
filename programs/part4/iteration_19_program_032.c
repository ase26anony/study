This code appears to be part of a loop nesting analysis algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps:

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no blocks in common**
   - If true: `continue` (skip to next iteration)

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if `other->block_bitmap` is a **subset** of `loop->block_bitmap`
   - `bitmap_intersect_compl_p(A, B)` returns true if there are any bits set in A that are NOT set in B
   - So `!bitmap_intersect_compl_p(A, B)` means "A has no bits that B doesn't have" → A ⊆ B
   - If true: Add `other` as a child/subloop of `loop`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks if `loop->block_bitmap` is a **subset** of `other->block_bitmap`
   - If true: Add `loop` as a child/subloop of `other`

## What it's determining

This code is building a loop nesting hierarchy by checking containment relationships:

- If `other` is completely contained within `loop` → `other` becomes a child of `loop`
- If `loop` is completely contained within `other` → `loop` becomes a child of `other`
- If neither is a subset of the other (they partially overlap) → no parent-child relationship is established
- If they don't intersect at all → skip (already handled by first condition)

## Example

Consider three loops with these blocks:
- Loop A: blocks {1, 2, 3, 4}
- Loop B: blocks {2, 3}
- Loop C: blocks {3, 4, 5}

The code would determine:
- B ⊆ A → B becomes child of A
- C ⊈ A and A ⊈ C (they share blocks 3,4 but A has 1,2 and C has 5) → no parent-child relationship

This is typical in compiler optimization passes that analyze loop structures for transformations like loop fusion, distribution, or parallelization.
