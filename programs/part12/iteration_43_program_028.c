This code appears to be part of a loop analysis or optimization pass in a compiler (likely GCC). Let me break down what this code is doing:

## Purpose
This code is establishing parent-child relationships between loops in a control flow graph.

## Logic Breakdown

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if `other` loop and `current loop` have **no common basic blocks**
   - If they have no blocks in common, `continue` (skip processing this `other` loop)

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if `other` loop's blocks are **completely contained within** `current loop`
   - `bitmap_intersect_compl_p(A, B)` returns true if A has any bits set that are NOT in B
   - The negation means: "if all of `other`'s blocks are in `loop`"
   - If true: `other` is a **child** of `loop`, so add `other` to `loop->loops`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks if `current loop`'s blocks are **completely contained within** `other` loop
   - If true: `loop` is a **child** of `other`, so add `loop` to `other->loops`

## What it's trying to determine:
- If two loops share some blocks (they intersect)
- Then figure out which one contains which (parent-child relationship)
- Build a hierarchy of nested loops

## Example:
If we have:
- Loop A contains blocks {1, 2, 3, 4}
- Loop B contains blocks {2, 3}

The code would determine that Loop B is a child of Loop A (B is nested inside A).

This is typical compiler code for building a loop tree structure during control flow analysis.
