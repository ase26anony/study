This code appears to be part of a loop nesting analysis or loop hierarchy construction algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps (sets of basic blocks):

1. **First check**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - This checks if the two loops have **no blocks in common**
   - If they don't share any blocks, `continue` (skip further processing)

2. **Second check**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if `A` has any bits that are NOT in `B`
   - So this checks if **all blocks of `other` are contained within `loop`**
   - If true, `other` is a **nested loop inside `loop`**
   - Add `other` to `loop->loops` (list of subloops)

3. **Third check**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks if **all blocks of `loop` are contained within `other`**
   - If true, `loop` is a **nested loop inside `other`**
   - Add `loop` to `other->loops` (list of subloops)

## What it's building

This code is constructing a **loop nesting tree** by determining parent-child relationships between loops based on their block containment:

- If loop A completely contains loop B → B is a child of A
- If loop B completely contains loop A → A is a child of B
- If loops partially overlap (neither completely contains the other) → no parent-child relationship is established (they might be sibling loops with some shared blocks)

## Example

Consider these loop block sets:
- Loop L1: blocks {1, 2, 3, 4, 5}
- Loop L2: blocks {2, 3, 4}

The code would determine that L2 is nested inside L1 because all blocks of L2 are in L1.
