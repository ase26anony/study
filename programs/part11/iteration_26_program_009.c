**Key Design Elements:**

1. **Decrementing Loop Patterns**: Each test function implements a decrementing loop with comparison against zero (`i != 0` or `i > 0`), matching the required RTL pattern.

2. **Volatile Variables**: Loop bounds are `volatile` to prevent constant propagation and early optimization.

3. **Inline Assembly**: `asm volatile("" : : "r"(i))` uses the loop counter as input, preventing its removal while not affecting the decrement pattern.

4. **Function Attributes**: `__attribute__((noinline, noclone))` preserves loop structure through compilation passes.

5. **Multiple Variants**: Different loop styles (for, while, unsigned, explicit decrement) increase coverage chances.

6. **Architecture-Specific Code**: Conditional compilation for ARM and MIPS targets hardware loop support.

7. **Side Effects**: `volatile` accumulators create observable behavior without breaking the decrement pattern.

**Compilation Examples:**
