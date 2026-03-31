This code appears to be part of a loop nesting analysis or control flow graph processing algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps:

1. **First check**: `bitmap_intersect_p(other->block_bitmap, loop->block_bitmap)`
   - Checks if the two loops share any basic blocks
   - If they don't intersect at all (`!bitmap_intersect_p`), `continue` to skip further processing

2. **Second check**: `bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)`
   - Checks if `other` has blocks that are NOT in `loop`
   - If `other` has NO blocks outside `loop` (meaning `other` is completely contained within `loop`), then add `other` as a nested loop inside `loop`

3. **Third check**: `bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)`
   - Checks if `loop` has blocks that are NOT in `other`
   - If `loop` has NO blocks outside `other` (meaning `loop` is completely contained within `other`), then add `loop` as a nested loop inside `other`

## What it's determining

This code is establishing parent-child relationships between loops:
- If loop A completely contains loop B → B is nested in A
- If loop B completely contains loop A → A is nested in B
- If loops partially overlap (both have blocks outside each other) → no nesting relationship is established

## Example

Consider two loops with these block sets:
- Loop A: blocks {1, 2, 3, 4}
- Loop B: blocks {2, 3}

The code would:
1. They intersect (share blocks 2, 3) → don't continue
2. Loop B has no blocks outside A → add B to A's nested loops
3. Loop A has blocks outside B → don't add A to B's nested loops

Result: B is nested inside A
