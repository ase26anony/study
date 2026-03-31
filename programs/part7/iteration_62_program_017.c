## Key Design Elements:

1. **Decrementing Loop Patterns**:
   - `test_for_loop_signed`: `for (int i = iterations; i != 0; i--)`
   - `test_while_loop_unsigned`: `while (n-- > 0)`
   - `test_do_while_decrement`: `do { ... } while (--count)`
   - `test_manual_decrement_unsigned`: `counter -= 1` inside body with `counter != 0` check
   - `test_nested_decrement`: `for (int j = base; j > 0; j--)`

2. **Register Pressure**:
   - Each function creates 5-8 temporary variables before the loop
   - `volatile` operations force register allocation
   - Additional volatile operations inside loop bodies

3. **Control Flow Inside Loops**:
   - `if/else` statements based on loop counter
   - Conditional `break`-like logic (resetting accumulators)
   - Modulo operations for branching decisions

4. **Post-Loop Counter Use**:
   - Each function adds the final counter value to the result
   - Results are combined in `main()` to prevent elimination

5. **Compilation Recommendations**:
