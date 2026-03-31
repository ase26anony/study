**Key design elements that target the uncovered lines:**

1. **Decrement-and-compare-to-zero patterns:**
   - `for (int i = n; i != 0; i--)` - Direct match for the RTL pattern
   - `while (i != 0) { ... i = i - 1; }` - Explicit decrement by 1
   - `for (unsigned int i = n; i != 0U; --i)` - Unsigned variant

2. **Volatile variables prevent early optimization:**
   - Function parameters marked `volatile`
   - Loop bounds from `volatile` arrays
   - Side effects through `global_counter` and `global_sink`

3. **Inline assembly preserves the counter:**
   - `asm volatile("" : "+r"(i) : : "memory")` - Forces register use
   - Prevents loop counter elimination or transformation

4. **Multiple variants increase coverage probability:**
   - Different loop styles (for, while, do-while)
   - Signed and unsigned counters
   - Nested loops to complicate analysis
   - Array accesses for memory side effects

5. **Compiler hints and attributes:**
   - `__attribute__((noinline, noclone))` - Preserves function boundaries
   - `optimize("O2")` and `optimize("Os")` - Different optimization levels
   - Architecture-specific functions for ARM/MIPS

6. **Execution flow with varying bounds:**
   - Multiple iterations with different loop counts
   - Checksum calculation prevents dead code elimination
   - Volatile updates ensure side effects

**Compilation recommendations:**
