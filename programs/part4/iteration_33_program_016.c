Looking at this code, it appears to be handling relationships between loops in a control flow graph. Let me break down what this code is doing:

## Code Analysis

The code checks relationships between two loops (`loop` and `other`) based on their block bitmaps:

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - This checks if the two loops have **no common blocks** at all
   - If they don't intersect at all, `continue` (skip further processing)

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if A has any bits that are NOT in B
   - So this condition is true when **all blocks of `other` are contained within `loop`**
   - If true: `loop->loops.safe_push(other)` - adds `other` as a subloop of `loop`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - This checks if **all blocks of `loop` are contained within `other`**
   - If true: `other->loops.safe_push(loop)` - adds `loop` as a subloop of `other`

## What This Code Does

This code is **building a loop nesting hierarchy** by determining parent-child relationships between loops:

- If `other` is completely inside `loop` → `other` becomes a child of `loop`
- If `loop` is completely inside `other` → `loop` becomes a child of `other`
- If neither is completely inside the other (they partially overlap or are disjoint after the first check) → no relationship is established

## Example Scenarios

1. **Nested loops**: Inner loop's blocks are a subset of outer loop's blocks
2. **Disjoint loops**: No common blocks (skipped by first condition)
3. **Partially overlapping loops**: Would pass the first check but fail both `intersect_compl_p` checks, so no parent-child relationship is established

This is typical in compiler optimization passes that analyze loop structures for transformations like loop unrolling, vectorization, or parallelization.
