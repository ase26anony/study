This program creates the specific patterns needed to trigger the uncovered code:

1. **Memory Address Patterns**: Multiple loops with `ptr++`, `ptr--`, and `ptr + stride` patterns
2. **Mixed Base Register Updates**: Sequences like `p = &arr[i]; val = *p;` followed by `p = p + 1;`
3. **Volatile and Non-Volatile Mix**: `volatile_pointer_walk` function uses volatile pointers
4. **Loop Variants**: Forward loops, reverse loops, while loops with pointer comparisons
5. **Function Boundaries**: Both inline (`always_inline`) and noinline functions
6. **Structure Access**: Nested structure access creates complex addressing that decomposes to base + offset

Compile with:
