## Key Design Elements:

1. **Decrement-to-Zero Pattern**: 
   - `while (i-- > 0)` generates post-decrement followed by comparison to zero
   - `for (; j > 0; j--)` generates separate decrement and compare instructions

2. **Volatile Bound Prevention**:
   - `volatile int N = 100` prevents constant propagation
   - Bound passed through function parameters maintains runtime variability

3. **Minimal Side Effects**:
   - `local_sum += 1` provides trivial computation
   - `asm volatile` statements prevent optimization without affecting the loop structure
   - Global variable `global_sum` ensures results are used

4. **Multiple Pattern Opportunities**:
   - Three different loops with similar decrement patterns
   - Different counter variables (`i`, `j`, `k`)
   - Nested loop context in `outer_wrapper`

5. **Architecture Considerations**:
   - Simple integer counters ensure register allocation
   - No pointer arithmetic or complex expressions in loop conditions

## Compilation Recommendations:
