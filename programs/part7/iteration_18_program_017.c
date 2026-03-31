This test program systematically covers all the required patterns:

1. **High-word patterns (A-D):**
   - Pattern A: `a.high = -1, b.high = 0` → unsigned: `0xFFFFFFFFFFFFFFFF > 0` → returns 1
   - Pattern B: `a.high = 0, b.high = -1` → unsigned: `0 < 0xFFFFFFFFFFFFFFFF` → returns -1
   - Pattern C: `a.high = 1, b.high = 2` → unsigned: `1 < 2` → returns -1
   - Pattern D: `a.high = -2, b.high = -3` → unsigned: `0xFFFFFFFFFFFFFFFE < 0xFFFFFFFFFFFFFFFD` → returns 1 (because -2 > -3)

2. **Low-word patterns (E-G):**
   - Pattern E: Equal high, `a.low = 1, b.low = 2` → returns -1
   - Pattern F: Equal high, `a.low = 2, b.low = 1` → returns 1
   - Pattern G: Equal both → returns 0

3. **Integration with GCC patterns:**
   - Uses `__int128` constants and operations
   - Uses GCC builtins (`__builtin_add_overflow`, `__builtin_mul_overflow`)
   - Complex loop conditions with `__int128` counters
   - Bitfield operations on wide types

4. **Optimization forcing:**
   - `__attribute__((noinline))` on comparison functions
   - `volatile` variables to prevent dead code elimination
   - Complex control flow that requires comparison results

**Compilation and execution:**
