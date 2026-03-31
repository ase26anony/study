This code appears to be part of a loop nesting analysis or loop hierarchy construction algorithm. Let me break down what it's doing:

## Purpose
The code is determining the relationship between two loops (`loop` and `other`) based on their block bitmaps (sets of basic blocks they contain).

## Logic Flow

1. **First check**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no blocks in common**
   - If true: `continue` (skip further processing for this `other` loop)

2. **Second check**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if `A` has any bits set that are **not** in `B`
   - So this checks: "Does `other` have any blocks that are **not** in `loop`?"
   - If **false** (no such blocks), then `other`'s blocks are a **subset** of `loop`'s blocks
   - Action: Add `other` as a child/subloop of `loop`

3. **Third check**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks: "Does `loop` have any blocks that are **not** in `other`?"
   - If **false** (no such blocks), then `loop`'s blocks are a **subset** of `other`'s blocks
   - Action: Add `loop` as a child/subloop of `other`

## What it's building
This code constructs a loop nesting tree where:
- If loop A completely contains loop B (B ⊆ A), then B becomes a child of A
- If loop B completely contains loop A (A ⊆ B), then A becomes a child of B
- If loops partially overlap (neither is a subset of the other), no parent-child relationship is established

## Example
Consider three loops with blocks:
- Loop1: {1, 2, 3, 4}
- Loop2: {2, 3} (nested inside Loop1)
- Loop3: {3, 4, 5} (partially overlaps with Loop1)

The code would:
1. Make Loop2 a child of Loop1 (Loop2 ⊆ Loop1)
2. Not establish parent-child between Loop1 and Loop3 (partial overlap)

This is typical in compiler loop analysis for constructing loop hierarchies in control flow graphs.
