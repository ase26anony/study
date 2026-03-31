This test program systematically covers all the required patterns:

1. **High-word patterns (A-D):**
   - Pattern A: Negative vs positive high word (tests `return 1` branch)
   - Pattern B: Positive vs negative high word (tests `return -1` branch)
   - Pattern C: Both positive, a.high < b.high (tests `return -1` branch)
   - Pattern D: Both negative with different magnitudes (tests both `return 1` and `return -1` branches)

2. **Low-word patterns (E-G):**
   - Pattern E: Equal high, a.low < b.low (tests `return -1` branch)
   - Pattern F: Equal high, a.low > b.low (tests `return 1` branch)
   - Pattern G: Equal both (tests fall-through to return 0)

3. **GCC integration patterns:**
   - Uses `__int128` operations that GCC internally represents as `double_int`
   - Uses GCC builtins (`__builtin_add_overflow`, `__builtin_mul_overflow`)
   - Creates complex loop conditions with `__int128` counters
   - Uses bitfield operations with wide types
   - Prevents optimization with `volatile` and `noinline`

**Compilation and execution:**
