This code appears to be part of a loop nesting analysis or loop hierarchy construction algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps and building a hierarchy:

1. **First check**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no blocks in common**
   - If they have no common blocks, `continue` (skip further processing)

2. **Second check**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if `other->block_bitmap` is a **subset** of `loop->block_bitmap`
   - `bitmap_intersect_compl_p(A, B)` returns true if `A & ~B` is non-empty
   - So `!bitmap_intersect_compl_p(A, B)` means `A & ~B` is empty, which means all bits in A are also in B (A ⊆ B)
   - If true: `loop->loops.safe_push(other)` - adds `other` as a child/subloop of `loop`

3. **Third check**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks if `loop->block_bitmap` is a **subset** of `other->block_bitmap`
   - If true: `other->loops.safe_push(loop)` - adds `loop` as a child/subloop of `other`

## What This Code Does

This code is **building a loop nesting tree** by comparing the basic blocks contained in each loop:

- If two loops share no blocks, they're unrelated (parallel/independent loops)
- If loop A's blocks are a subset of loop B's blocks, then A is nested inside B
- If loop B's blocks are a subset of loop A's blocks, then B is nested inside A
- If neither is a subset of the other but they share blocks, they might be partially overlapping (which might indicate an issue in the loop analysis)

## Example

Consider three loops with these block sets:
- Loop A: blocks {1, 2, 3, 4}
- Loop B: blocks {2, 3}
- Loop C: blocks {5, 6}

The code would:
- Compare A and B: B ⊆ A, so add B as child of A
- Compare A and C: no intersection, skip
- Compare B and C: no intersection, skip

This appears to be from GCC or a similar compiler's loop analysis infrastructure, where `block_bitmap` represents the set of basic blocks belonging to each loop.
