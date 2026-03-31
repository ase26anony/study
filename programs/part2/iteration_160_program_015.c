This program creates the specific loop relationships needed:

1. **Subset Relationships**: `test_subset_relationship()` creates loops where inner loops are fully contained within outer loops (B inside A).

2. **Intersecting Non-Subset Loops**: `test_intersecting_loops()` creates loops D and E that share blocks but each has unique blocks outside the other.

3. **Disjoint Loops**: `test_disjoint_loops()` creates loops F and G with no block intersection.

4. **Complex Control Flow**: Uses `goto`, `switch`, computed goto, early exits, function calls, and memory barriers to create complex basic block patterns.

5. **Hardware Loop Patterns**: Uses counted loops with constant bounds, array operations, and mixed integer/float computations.

6. **Architecture Targeting**: Uses `__attribute__((target(...)))` for PowerPC and ARM backends.

**Compilation recommendations:**
