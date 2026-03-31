This code appears to be part of a loop nesting analysis or loop hierarchy construction algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps and building a hierarchy:

1. **First check**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - This checks if the two loops have **no blocks in common**
   - If they have no common blocks, `continue` to skip further processing

2. **Second check**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if A has any bits that are **not** in B
   - So this checks if `other->block_bitmap` is **completely contained within** `loop->block_bitmap`
   - If true, `other` is a **sub-loop** of `loop`, so add `other` to `loop->loops`

3. **Third check**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks if `loop->block_bitmap` is **completely contained within** `other->block_bitmap`
   - If true, `loop` is a **sub-loop** of `other`, so add `loop` to `other->loops`

## What it's Building

This code is constructing a **loop nesting tree** where:
- Each loop maintains a list (`loops`) of its immediate sub-loops
- The relationships are determined by block containment:
  - If loop A contains all blocks of loop B, then B is a sub-loop of A
  - If they partially overlap (neither completely contains the other), no parent-child relationship is established

## Example Scenario

Consider three loops with these block sets:
- Loop A: blocks {1, 2, 3, 4, 5}
- Loop B: blocks {2, 3, 4}
- Loop C: blocks {3, 4, 6}

The code would establish:
- B is a sub-loop of A (B's blocks are all in A)
- No relationship between A and C (they overlap but neither contains the other)
- No relationship between B and C (they overlap but neither contains the other)

This is typical in compiler optimization passes that need to understand loop nesting for transformations like loop-invariant code motion, loop unrolling, or parallelization.
