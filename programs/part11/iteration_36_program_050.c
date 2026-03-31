This program creates the required loop relationships:

1. **Loop A (in `process_loop_a`)** is an outer loop that contains:
   - Multiple basic blocks (A1-A8)
   - Loop B fully contained within one branch (A2)
   - Partial overlap with Loop C via `shared_computation()`

2. **Loop B** is fully contained within Loop A's true branch, so `bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)` should return false for B relative to A.

3. **Loop C (in `process_loop_c`)** shares blocks with Loop A through `shared_computation()` but also has unique blocks (C2-C5), creating the intersecting but not contained relationship.

The program uses:
- `volatile` variables to prevent optimization
- `__attribute__((noinline))` to preserve function boundaries
- Complex conditionals to create distinct basic blocks
- Early exits (`break`, `return`) to create additional control flow edges
- Shared functions to create bitmap intersections
- Goto statements for irreducible flow
- Command-line arguments for dynamic loop bounds

To compile and test for coverage:
