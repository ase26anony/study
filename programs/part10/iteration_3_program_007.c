This code appears to be part of a loop nesting analysis algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps and building a loop hierarchy.

### Step-by-step logic:

1. **First check**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no blocks in common**
   - If true: `continue` (skip this `other` loop entirely)
   - This means we only process loops that share at least one block

2. **Second check**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if there are any bits set in A that are **not** set in B
   - So this checks: Are **all** blocks of `other` also in `loop`?
   - If true: `other` is completely contained within `loop`
   - Action: Add `other` as a child/subloop of `loop`

3. **Third check**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks: Are **all** blocks of `loop` also in `other`?
   - If true: `loop` is completely contained within `other`
   - Action: Add `loop` as a child/subloop of `other`

## What this builds:

This code constructs a **loop nesting tree** by determining containment relationships between loops:

- If loop A contains all blocks of loop B → B is nested inside A
- If loop B contains all blocks of loop A → A is nested inside B
- If they partially overlap but neither fully contains the other → no parent-child relationship is established (they might be sibling loops or have some other complex relationship)

## Example scenario:

Consider three basic blocks and two loops:
- Loop X: blocks {1, 2, 3}
- Loop Y: blocks {2, 3}

The code would determine that Loop Y is nested inside Loop X because all of Y's blocks are in X.

This is typical in compiler optimization passes that need to understand loop hierarchies for transformations like loop-invariant code motion, loop unrolling, or parallelization.
