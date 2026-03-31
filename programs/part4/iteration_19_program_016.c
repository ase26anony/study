This code appears to be part of a loop nesting analysis algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps and building a loop nesting hierarchy.

### Step-by-step logic:

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no blocks in common**
   - If they share no blocks, `continue` (skip further processing for this `other` loop)

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if A has any bits that are NOT in B
   - The negation `!bitmap_intersect_compl_p(A, B)` means: **All bits in A are also in B**
   - So this checks: Are all blocks of `other` also in `loop`?
   - If YES: `other` is completely contained within `loop` → add `other` as a child of `loop`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks: Are all blocks of `loop` also in `other`?
   - If YES: `loop` is completely contained within `other` → add `loop` as a child of `other`

## What it's building:

This code constructs a **loop tree** where:
- Parent loops contain all blocks of their child loops
- The relationships are stored in `loops.safe_push()` calls
- Each loop's `loops` member appears to be a list of its immediate subloops

## Example scenario:

If we have:
- Loop A with blocks {1, 2, 3, 4}
- Loop B with blocks {2, 3}

The code would:
1. They intersect (share blocks 2, 3) → don't skip
2. All blocks of B are in A → add B as child of A
3. Not all blocks of A are in B → skip the else-if

Result: A becomes parent of B in the loop hierarchy.

This is typical in compiler optimization passes for analyzing loop nesting structures in control flow graphs.
