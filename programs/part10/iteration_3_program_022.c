This code appears to be part of a loop nesting analysis algorithm. Let me break down what it's doing:

## Purpose
This code is determining the nesting relationship between two loops (`loop` and `other`) based on their block bitmaps (sets of basic blocks).

## Logic Breakdown

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no blocks in common**
   - If true: `continue` - skip further processing (loops don't intersect at all)

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if `A` has any bits that are NOT in `B`
   - So this checks: "Does `other` have any blocks that are NOT in `loop`?"
   - If NOT (i.e., `other` has NO blocks outside `loop`), then `other` is **completely contained within** `loop`
   - Result: Add `other` as a child/subloop of `loop`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks the opposite: "Does `loop` have any blocks that are NOT in `other`?"
   - If NOT (i.e., `loop` has NO blocks outside `other`), then `loop` is **completely contained within** `other`
   - Result: Add `loop` as a child/subloop of `other`

## What it's trying to determine:
This code is building a loop nesting tree by checking containment relationships:
- If loops share blocks (first condition passes)
- Then check which loop contains which:
  - If `other ⊆ loop` → `other` is nested in `loop`
  - If `loop ⊆ other` → `loop` is nested in `other`
  - If neither is true (both have blocks outside the other), they're siblings at the same nesting level

## Edge Cases:
- The code doesn't handle the case where loops partially overlap but neither contains the other completely (they remain unrelated in the nesting tree)
- It assumes loops are properly nested (no irreducible loops or overlapping loops at the same level)

This is typical of compiler optimization passes that analyze loop structures for transformations like loop-invariant code motion, vectorization, or parallelization.
