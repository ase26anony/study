This program creates the specific patterns needed to trigger the uncovered code:

1. **Memory Address Patterns**: Multiple loops with `ptr++`, `ptr--`, and pointer arithmetic create the linear access patterns.

2. **Mixed Base Register Updates**: Functions like `pointer_arithmetic_explicit` explicitly create the `p = &arr[i]; val = *p;` pattern followed by increment.

3. **Volatile and Non-Volatile Mix**: `process_with_volatile` and `mixed_volatile_access` use volatile pointers alongside regular ones.

4. **Loop Variants**: Forward loops, backward loops, while loops with pointer comparisons, and loops with different strides (1, 2, sizeof(Data)).

5. **Function Inlining Boundaries**: Mix of `static` functions (likely inlined) and `noinline` functions create different optimization contexts.

6. **Structure and Array Combination**: `process_struct_array` and `process_nested_structure` access structure fields with constant offsets within array traversal.

The key pattern that should trigger lines 1352-1358 is when the compiler's RTL generation creates memory instructions with `reg1_is_const = true` and `reg1_val = 0`, followed by an increment instruction that `find_inc(true)` can merge into an auto-increment addressing mode.

Compile with:
