**Key design elements targeting the uncovered lines:**

1. **Memory Address Patterns**: Multiple loops with `*ptr++`, `*ptr--`, and explicit `ptr = ptr + 1` updates create the linear addressing patterns needed for auto-increment detection.

2. **Mixed Base Register Updates**: Functions like `combined_test()` explicitly separate the load (`int temp = *ptr`) from the pointer update (`ptr = ptr + 1`), creating the exact pattern where `find_inc(true)` would analyze if the increment can be merged.

3. **Volatile and Non-Volatile Mix**: `volatile_walk()` uses volatile pointers while other functions use regular pointers, testing the pass's ability to handle both.

4. **Constant Stride Loops**: All loops use constant stride of 1 (for int arrays) or `sizeof(Data)` (for structure arrays).

5. **Inlining Boundaries**: `mixed_access_pattern()` is marked `noinline` while `inline_helper()` is marked `inline`, creating different optimization contexts.

6. **Structure/Array Combination**: The `Data` structure with multiple fields accessed via `ptr->field` creates complex addressing that decomposes to base + constant offset patterns.

**Compilation recommendations:**
