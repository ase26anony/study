**Key patterns that should trigger the uncovered code:**

1. **Memory Address Patterns**: Multiple loops with `ptr++` and `ptr--` operations create linear addressing patterns that the auto-inc-dec pass can analyze.

2. **Mixed Base Register Updates**: Functions like `multiple_updates_in_block` explicitly create sequences where a base register is used for memory access and then immediately updated, which should create the `mem_insn` structure with `reg1_val = 0`.

3. **Volatile and Non-Volatile Mix**: The `mixed_volatile_access` function and volatile pointer walks create both volatile and non-volatile access patterns for the pass to analyze.

4. **Loop Variants**: Multiple loop styles (`for`, `while`, forward, backward) with constant strides of ±1 create the RTL patterns needed.

5. **Inlining Boundaries**: Using `noinline` and `always_inline` attributes ensures both inlined and non-inlined versions are tested.

6. **Structure/Array Combination**: Accessing `struct_array[i].field` creates complex addressing that decomposes to base+offset patterns.

**Compilation recommendations:**
