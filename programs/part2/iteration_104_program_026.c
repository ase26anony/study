**Key design elements that target the uncovered code:**

1. **Decrement-and-Compare Patterns:**
   - `while (i-- > 0)` - Classic post-decrement compare-to-zero
   - `for (; count; --count)` - Pre-decrement with implicit compare
   - `while (--n > 0)` - Pre-decrement in do-while
   - `while (k) { ... k--; }` - Separate decrement and compare

2. **Integer Types:** All loop counters are plain `int` to ensure RTL generates `PLUS` with `-1` and `COMPARE` with `const0_rtx`.

3. **Preventing Optimization:**
   - `volatile` variables prevent constant propagation
   - `asm volatile("" : : : "memory")` creates side effects
   - Results are accumulated in `global_counter` and printed

4. **Nesting Context:** 
   - `test_loops()` is called from `outer_loop()`
   - Multiple calls in `main()` create different optimization contexts
   - This increases chances the doloop pass analyzes the pattern

5. **Non-Constant Bounds:** Loop bounds come from `argc`/`argv` or volatile variables, preventing compile-time trip count computation.

**Compilation recommendations:**
