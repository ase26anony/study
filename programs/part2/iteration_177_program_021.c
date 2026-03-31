**Key design elements that target the uncovered lines:**

1. **Decrement-to-Zero Pattern**: The loops use `i-- > 0`, `j--`, and `--k > 0` patterns which should generate RTL sequences with `(plus reg -1)` followed by comparison against zero.

2. **Volatile Loop Bounds**: `N` and `M` are declared `volatile` to prevent constant propagation and loop unrolling/elimination.

3. **Minimal Side Effects**: 
   - Assignment to `global_side_effect` (volatile)
   - Inline assembly with memory clobber
   - Accumulation into `global_sum`

4. **Multiple Loop Patterns**: Four different loops with similar but not identical patterns to give the optimizer multiple matching opportunities.

5. **Nested Loops**: `test_loop_pattern2` contains nested loops to increase the chance the do-loop optimizer runs.

**Compilation options to verify the pattern matches:**
