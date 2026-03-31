**Key design elements that target the uncovered code:**

1. **Decrement-and-Compare Patterns:**
   - `while (i-- > 0)` - Classic post-decrement compare-to-zero
   - `for (; count; --count)` - Pre-decrement with implicit compare-to-zero
   - `do { ... } while (n-- > 0)` - Post-decrement at loop bottom

2. **Integer Types:** All loop counters are plain `int` to ensure RTL generates `PLUS` with `-1` and `COMPARE` with `const0_rtx`.

3. **Preventing Optimization:**
   - `volatile` accumulators prevent dead code elimination
   - `asm volatile("" : : : "memory")` creates memory barriers
   - Non-constant bounds from command line arguments
   - Multiple loop calls prevent inlining and constant propagation

4. **Nesting Context:** `loop4()` has an outer loop to provide optimization context, and `main()` calls all loops multiple times.

5. **Compilation Recommendations:**
