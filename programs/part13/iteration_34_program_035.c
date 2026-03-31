This code appears to be part of a loop nesting analysis algorithm. Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps and building a loop hierarchy.

### Step-by-step logic:

1. **First check**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no blocks in common**
   - If they have no common blocks, `continue` (skip further processing)

2. **Second check**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if A has any bits that are NOT in B
   - So this checks: "Does `other` have any blocks that are NOT in `loop`?"
   - If NOT (meaning all blocks of `other` are contained in `loop`), then:
     - `loop->loops.safe_push(other)` - Add `other` as a child/sub-loop of `loop`

3. **Third check**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks: "Does `loop` have any blocks that are NOT in `other`?"
   - If NOT (meaning all blocks of `loop` are contained in `other`), then:
     - `other->loops.safe_push(loop)` - Add `loop` as a child/sub-loop of `other`

## What this means:

This code is building a **loop nesting tree** by determining parent-child relationships between loops:

- If `other` is completely inside `loop` → `other` becomes child of `loop`
- If `loop` is completely inside `other` → `loop` becomes child of `other`
- If neither is completely inside the other (they partially overlap or are disjoint) → no parent-child relationship is established

## Example scenarios:

1. **Nested loops**: Inner loop's blocks are a subset of outer loop's blocks
2. **Disjoint loops**: No common blocks (skipped by first check)
3. **Partially overlapping loops**: Would pass first check but fail both `intersect_compl_p` checks, so no relationship established

This is typical in compiler control flow analysis for determining loop hierarchy in programs with nested loops.
