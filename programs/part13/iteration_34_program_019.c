This code appears to be part of a loop nesting analysis algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps:

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no common blocks** at all
   - If true, `continue` - skip further processing for this `other` loop

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if `other->block_bitmap` is a **subset** of `loop->block_bitmap`
   - `bitmap_intersect_compl_p(A, B)` returns true if `A & ~B` is non-empty
   - So `!bitmap_intersect_compl_p(A, B)` means `A & ~B` is empty → all bits in A are also in B → A ⊆ B
   - If true: `other` is nested inside `loop`, so add `other` to `loop->loops`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks if `loop->block_bitmap` is a **subset** of `other->block_bitmap`
   - If true: `loop` is nested inside `other`, so add `loop` to `other->loops`

## What it's doing

This code is building a **loop nesting tree** by determining parent-child relationships between loops:

- If `other` ⊆ `loop` → `other` is a child of `loop`
- If `loop` ⊆ `other` → `loop` is a child of `other`
- If neither is a subset of the other but they intersect → they're siblings or have some other relationship

## Example

Consider three loops with these blocks:
- Loop A: blocks {1, 2, 3, 4}
- Loop B: blocks {2, 3}
- Loop C: blocks {3, 4, 5}

For A and B:
- They intersect (blocks 2, 3)
- B ⊆ A → B is child of A

For A and C:
- They intersect (blocks 3, 4)
- Neither is subset of the other → no parent-child relationship recorded here

For B and C:
- They intersect (block 3)
- Neither is subset of the other → no parent-child relationship

This is typical in compiler optimization passes that analyze loop structures for transformations like loop unrolling, vectorization, or parallelization.
