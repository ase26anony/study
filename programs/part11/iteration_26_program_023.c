**Key Design Elements:**

1. **Decrement Patterns**: Each test function implements a slightly different variant of the decrement-and-compare-to-zero pattern:
   - `i != 0` condition (most direct match)
   - `i > 0` condition
   - `while (i--)` pattern
   - Explicit `i = i - 1` decrement
   - Unsigned counter variant

2. **Optimization Barriers**:
   - `volatile` loop bounds prevent compile-time unrolling
   - `asm volatile("" : : "r"(i))` prevents loop counter optimization
   - `__attribute__((noinline, noclone))` keeps functions intact
   - `volatile` accumulators prevent dead code elimination

3. **Multiple Test Cases**: Different loop structures increase the chance that one will match the exact RTL pattern during the doloop optimization pass.

4. **Architecture-Specific Code**: Conditional compilation for ARM and MIPS targets where hardware loop support is most relevant.

**Compilation Recommendations:**
