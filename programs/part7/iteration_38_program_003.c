This code appears to be part of a loop nesting analysis algorithm, likely from a compiler optimization pass (like GCC's loop infrastructure). Let me break down what it's doing:

## Code Analysis

The code is checking relationships between two loops (`loop` and `other`) based on their block bitmaps and building a loop hierarchy.

### Step-by-step logic:

1. **First check**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - Checks if the two loops have **no blocks in common**
   - If they share no blocks, `continue` (skip further processing)

2. **Second check**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if `A` has any bits that are NOT in `B`
   - So this checks: "Does `other` have any blocks that are NOT in `loop`?"
   - If `other` has **no blocks outside** `loop`, then `other` is completely contained within `loop`
   - In this case: `loop->loops.safe_push(other)` - adds `other` as a child/subloop of `loop`

3. **Third check**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - Checks: "Does `loop` have any blocks that are NOT in `other`?"
   - If `loop` has **no blocks outside** `other`, then `loop` is completely contained within `other`
   - In this case: `other->loops.safe_push(loop)` - adds `loop` as a child/subloop of `other`

## What it's building:

This code is constructing a **loop nesting tree** where:
- Each loop maintains a list of its immediate subloops (`loops` vector)
- The algorithm determines parent-child relationships based on block containment

## Example scenarios:

1. **Disjoint loops**: Share no blocks → skip (first `if`)
2. **`other` inside `loop`**: All blocks of `other` are in `loop` → `other` becomes child of `loop`
3. **`loop` inside `other`**: All blocks of `loop` are in `other` → `loop` becomes child of `other`
4. **Overlapping but not nested**: Both have blocks outside each other → neither condition matches, no relationship established

## Important note:

The `else if` ensures that only one direction of nesting is recorded. If two loops are exactly identical (same block set), the second condition would be true, making `loop` a child of `other`.

This is typical compiler code for building the loop tree structure used in optimization passes like loop-invariant code motion, loop unrolling, etc.
