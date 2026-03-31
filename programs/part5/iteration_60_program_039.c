Looking at this code, it appears to be part of a loop analysis or loop nesting structure algorithm. Let me break down what this code is doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps:

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - This checks if the two loops have **no common basic blocks** at all
   - If they don't intersect at all, `continue` to skip further processing

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if A has any bits that are NOT in B
   - So this condition is true when **all blocks of `other` are contained within `loop`**
   - If true: `loop->loops.safe_push(other)` - adds `other` as a child/subloop of `loop`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - This checks if **all blocks of `loop` are contained within `other`**
   - If true: `other->loops.safe_push(loop)` - adds `loop` as a child/subloop of `other`

## What This Code Does

This code is **building a loop nesting tree** by determining parent-child relationships between loops:

- If `other` is completely inside `loop` → `other` becomes a child of `loop`
- If `loop` is completely inside `other` → `loop` becomes a child of `other`
- If neither is completely inside the other (they partially overlap or are disjoint) → no parent-child relationship is established

## Important Notes

1. The `continue` at the beginning means loops that don't share any blocks are ignored entirely
2. The logic assumes that two loops either:
   - Don't intersect at all (skipped)
   - One is completely contained within the other (parent-child relationship)
   - Partially overlap (no relationship established in this code)

3. This appears to be part of a compiler optimization pass (likely GCC's loop infrastructure) where loops are represented as sets of basic blocks

The code is essentially determining the nesting structure of loops in a control flow graph based on which basic blocks they contain.
