**Key features that address the requirements:**

1. **Constant Bounds**: Uses `constexpr` template functions (`constant<N>()`) to generate compile-time constant indices.

2. **MEM_P vs non-MEM_P paths**:
   - `test_mem_small_count`, `test_mem_large_count_*`: Memory-to-memory operations (trigger `MEM_P`)
   - `test_non_mem`: Results assigned to register variable (non-`MEM_P`)

3. **Count values**:
   - `count = 1`: Case 1 with `lo=10, hi=10`
   - `count = 2`: Case 0 with `lo=5, hi=6`
   - `count > 2`: Cases 2, 3, 4 with counts of 6 and 8

4. **Element sizes**:
   - `char`: Small TYPE_SIZE (1 byte)
   - `short`: Intermediate size (2 bytes typically)
   - `long long`: Larger size (8 bytes typically)

5. **Volatile usage**: `volatile int dummy` and `volatile` pointers inhibit early constant folding.

6. **Builtin memcpy**: Uses `__builtin_memcpy` with constant sizes.

7. **Multiple paths**: `argc` switch selects different test scenarios.

**Compilation recommendations:**
