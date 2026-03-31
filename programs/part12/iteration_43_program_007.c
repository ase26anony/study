This code appears to be part of a loop nesting analysis algorithm, likely from a compiler optimization pass (like GCC's loop infrastructure). Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps and building a loop nesting hierarchy.

### Step-by-step logic:

1. **First check**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no blocks in common**
   - If they have no common blocks, `continue` (skip further processing)

2. **Second check**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if A has any bits that are NOT in B
   - So this checks: "Does `other` have any blocks that are NOT in `loop`?"
   - If `other` has **NO blocks outside** of `loop`, then `other` is **completely contained within** `loop`
   - In this case: `loop->loops.safe_push(other)` - add `other` as a child/subloop of `loop`

3. **Third check**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks: "Does `loop` have any blocks that are NOT in `other`?"
   - If `loop` has **NO blocks outside** of `other`, then `loop` is **completely contained within** `other`
   - In this case: `other->loops.safe_push(loop)` - add `loop` as a child/subloop of `other`

## What it's building:

This code is constructing a **loop nesting tree** where:
- Each loop maintains a list of its immediate subloops (`loops` vector)
- The algorithm determines containment relationships between loops
- If loop A completely contains loop B, then B becomes a child of A
- If loops partially overlap (each has some blocks the other doesn't), neither is added as a child (they're considered siblings or overlapping in some other way)

## Example:

Consider these loops and their blocks:
- Loop L1: blocks {1, 2, 3, 4}
- Loop L2: blocks {2, 3}

For L1 and L2:
1. They intersect (share blocks 2, 3) ✓
2. L2 has no blocks outside L1 (2,3 are in L1) → L2 is child of L1
3. L1 DOES have blocks outside L2 (1,4) → not the else case

Result: L2 becomes a subloop of L1

This is typical in compiler loop analysis to understand loop nesting for optimizations like loop-invariant code motion, vectorization, etc.
