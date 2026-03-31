Looking at this code, it appears to be handling relationships between loops in a control flow graph. Let me break down what each condition is checking:

1. **First condition**: `if (!bitmap_intersect_p (other->block_bitmap, loop->block_bitmap))`
   - This checks if the two loops have **no common basic blocks** at all
   - If they don't share any blocks, `continue` skips further processing

2. **Second condition**: `if (!bitmap_intersect_compl_p (other->block_bitmap, loop->block_bitmap))`
   - `bitmap_intersect_compl_p(A, B)` checks if A has any bits set that are **not** set in B
   - So this checks if `other->block_bitmap` is a **subset** of `loop->block_bitmap`
   - If true, `other` is completely contained within `loop`, so `other` is added as a child of `loop`

3. **Third condition**: `else if (!bitmap_intersect_compl_p (loop->block_bitmap, other->block_bitmap))`
   - This checks if `loop->block_bitmap` is a **subset** of `other->block_bitmap`
   - If true, `loop` is completely contained within `other`, so `loop` is added as a child of `other`

**What this code is doing**: It's building a loop hierarchy (nesting structure) by determining containment relationships between loops based on their basic block sets.

**Important notes**:
- The first `if` ensures we only process loops that share at least one block (which is necessary for containment relationships)
- The `else if` ensures mutual exclusivity - a loop can't be both a subset and superset of another (unless they're equal, which would be a different case)
- This assumes loops are properly nested (no partial overlaps where each has blocks the other doesn't have)

**Potential edge case**: If two loops have exactly the same set of basic blocks, neither condition would be true (both `bitmap_intersect_compl_p` calls would return true), so no parent-child relationship would be established. This might indicate an issue in loop detection or require special handling.
