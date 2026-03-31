This code appears to be part of a loop nesting analysis algorithm, likely from a compiler optimization pass (like GCC). Let me break down what it's doing:

## What the code does:

1. **First check**: `bitmap_intersect_p(other->block_bitmap, loop->block_bitmap)`
   - Checks if the two loops share any basic blocks
   - If they don't intersect at all (`!bitmap_intersect_p` returns true), skip to next iteration with `continue`

2. **Second check**: `bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)`
   - Checks if `other->block_bitmap` has any bits set that are NOT in `loop->block_bitmap`
   - If `other` is completely contained within `loop` (no bits outside), then `other` is a child of `loop`

3. **Third check**: `bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)`
   - Checks if `loop->block_bitmap` has any bits set that are NOT in `other->block_bitmap`
   - If `loop` is completely contained within `other`, then `loop` is a child of `other`

## In plain English:

This code determines the parent-child relationship between two loops based on their basic block sets:

1. If loops don't share any blocks → ignore (continue)
2. If loop `other` is completely inside loop `loop` → `other` becomes a child of `loop`
3. If loop `loop` is completely inside loop `other` → `loop` becomes a child of `other`

## Example:
- Loop A has blocks: {1, 2, 3, 4}
- Loop B has blocks: {2, 3}
- Result: Loop B is a child of Loop A (case 2)

## Note:
The `bitmap_intersect_compl_p(a, b)` function returns true if `a` has any bits that are NOT in `b`. So when it returns false, it means all bits in `a` are also in `b` (a ⊆ b).

This is building a loop tree structure where loops can be nested inside other loops.
