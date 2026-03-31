This code appears to be part of a loop nesting analysis or loop hierarchy construction algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps (sets of basic blocks):

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no common blocks** (bitmaps don't intersect)
   - If true: `continue` - skip further processing (loops are disjoint)

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if `other->block_bitmap` is a **subset** of `loop->block_bitmap`
   - `bitmap_intersect_compl_p(A, B)` returns true if `A & ~B` is non-empty
   - So `!bitmap_intersect_compl_p(A, B)` means `A & ~B` is empty → all blocks in A are also in B → A ⊆ B
   - If true: `other` is nested inside `loop`, so add `other` to `loop->loops`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks if `loop->block_bitmap` is a **subset** of `other->block_bitmap`
   - If true: `loop` is nested inside `other`, so add `loop` to `other->loops`

## What it's building

This code is constructing a **loop nesting tree** by determining parent-child relationships between loops based on their block containment:

- If loops share no blocks: they're unrelated (siblings or cousins at best)
- If one loop's blocks are completely contained within another: it's a nested loop
- The code adds child loops to their parent's `loops` list

## Example

Consider three loops with these block sets:
- Loop A: {1, 2, 3, 4}
- Loop B: {2, 3}
- Loop C: {5, 6}

Processing A vs B:
- They intersect (share blocks 2, 3)
- B ⊆ A is true → Add B to A.loops

Processing A vs C:
- No intersection → skip (continue)

Processing B vs C:
- No intersection → skip
