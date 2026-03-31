This code appears to be part of a loop nesting analysis algorithm, likely from a compiler optimization pass (like GCC's loop infrastructure). Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps:

1. **First check**: `bitmap_intersect_p(other->block_bitmap, loop->block_bitmap)`
   - Checks if the two loops share any basic blocks
   - If they don't intersect at all, `continue` (skip further processing)

2. **Second check**: `bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)`
   - Checks if `other` has blocks that are NOT in `loop`
   - If `other` is completely contained within `loop` (all blocks of `other` are in `loop`), then `other` is a nested loop inside `loop`

3. **Third check**: `bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)`
   - Checks if `loop` has blocks that are NOT in `other`
   - If `loop` is completely contained within `other`, then `loop` is a nested loop inside `other`

## What it's determining

This code is building a loop nesting hierarchy by:
- Identifying when one loop is completely contained within another
- Adding the inner loop to the outer loop's list of nested loops

## Example scenario

Consider two loops:
- Loop A: blocks {1, 2, 3, 4}
- Loop B: blocks {2, 3}

The code would:
1. See they intersect (share blocks 2, 3)
2. Check if B has blocks not in A → B's blocks {2,3} are all in A → B is nested in A
3. Add B to A's nested loops list

## Potential issues

The logic seems to assume mutual exclusivity (either A contains B or B contains A), but what if they partially overlap without one containing the other? In that case, neither condition would be true, and no relationship would be established.

This is likely correct for well-structured loops in valid programs, where loops should either be disjoint or properly nested.
