**Key design elements that target the uncovered lines:**

1. **Decrement-to-Zero Pattern**: 
   - `while (i-- > 0)` - Post-decrement creates the `(plus reg -1)` pattern
   - `for (; j > 0; j--)` - Separate decrement followed by comparison
   - `while (--k > 0)` - Pre-decrement in do-while context

2. **Volatile Loop Bounds**:
   - `volatile int N = 100` prevents constant propagation
   - Function parameters passed from volatile sources

3. **Minimal Side Effects**:
   - `local_sum += 1` provides a trivial side effect
   - `asm volatile("" : : : "memory")` prevents dead code elimination
   - Global variable `global_sum` ensures results are used

4. **Multiple Loop Candidates**:
   - Two test functions with similar patterns
   - Multiple loops in each function with different counter names
   - Outer loop in `main()` to increase optimization opportunities

5. **Avoidance of Complex Patterns**:
   - Simple integer counters (no pointers)
   - Direct decrement operations
   - Comparisons against zero

**Compilation options to verify the pattern matches:**
