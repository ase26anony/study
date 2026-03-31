## Key Design Elements:

1. **Decrement-and-Compare Patterns**: The code includes 5 different loop patterns that should generate the target RTL:
   - `while (i-- > 0)` - Classic post-decrement compare
   - `for (; count; --count)` - Pre-decrement in loop update
   - `do { ... } while (--n > 0)` - Decrement at loop bottom
   - `while (j) { ... j--; }` - Separate decrement
   - `for (; k--; )` - Post-decrement in condition

2. **Non-Constant Loop Bounds**: Uses `argc` and `atoi()` to make loop counts variable at compile time, preventing constant propagation.

3. **Volatile and Side Effects**:
   - `volatile int acc` prevents elimination of accumulator updates
   - `asm volatile("" : : : "memory")` creates memory barriers
   - `global_accumulator` ensures results are used

4. **Nested Context**: 
   - `test_loops()` contains multiple loop patterns
   - `outer_loop()` provides outer loop context
   - Multiple calls in `main()` increase optimization opportunities

5. **Integer Types**: All loop counters are plain `int` to match the expected RTL pattern.

## Compilation Recommendations:
