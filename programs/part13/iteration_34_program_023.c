Looking at this code, it appears to be handling relationships between loops in a control flow graph. Let me break down what this code is doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps:

1. **First check**: `bitmap_intersect_p(other->block_bitmap, loop->block_bitmap)`
   - Checks if the two loops share any basic blocks
   - If they don't intersect at all, `continue` (skip further processing)

2. **Second check**: `bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)`
   - Checks if `other` has any blocks that are NOT in `loop`
   - If `other` has NO blocks outside `loop` (i.e., `other` is completely contained within `loop`), then `other` is a nested loop inside `loop`

3. **Third check**: `bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)`
   - Checks if `loop` has any blocks that are NOT in `other`
   - If `loop` has NO blocks outside `other` (i.e., `loop` is completely contained within `other`), then `loop` is a nested loop inside `other`

## What the code does:

This code is building a loop hierarchy/nesting structure:
- If `other` is completely contained within `loop`, add `other` to `loop->loops` (as a nested loop)
- If `loop` is completely contained within `other`, add `loop` to `other->loops` (as a nested loop)

## Important note:

The conditions are mutually exclusive when both loops intersect but neither is completely contained within the other (they overlap partially), neither `safe_push` operation will execute.

## Example scenario:

Consider two loops A and B:
- If all blocks of B are within A → B is nested in A
- If all blocks of A are within B → A is nested in B
- If they share some blocks but each has blocks the other doesn't have → they're neither nested nor the same loop (might be sibling loops with shared header or other complex relationship)

This is typical in compiler optimization passes that analyze loop structures for transformations like loop fusion, distribution, or parallelization.
