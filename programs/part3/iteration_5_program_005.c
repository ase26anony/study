**Key design elements that address the requirements:**

1. **Constant bounds via `constexpr`**: The `const_index<N>()` template function ensures indices are compile-time constants visible to the middle-end.

2. **`MEM_P` and non-`MEM_P` paths**: 
   - Memory targets: Array copies (`v_char[i] = v_char[i + 10]`)
   - Non-memory targets: Register operations (`sum += v_int[i]`, `product = v_int[idx_a] * v_int[idx_b]`)

3. **Controlled `count` values**:
   - `count = 1/2`: Scenarios with indices like `[5..6]` or single element `[10]`
   - `count > 2`: Ranges like `[20..30]` (count=11) with different element sizes

4. **Volatile inhibition**: `volatile` pointers and `volatile_wrapper` prevent early constant folding while allowing the middle-end to still recognize constant bounds.

5. **Array block operations**: Loops copying subranges and `__builtin_memcpy` with constant sizes.

6. **Multi-path reachability**: Conditional branches based on `argc` ensure different code paths are analyzed.

**Compilation recommendations:**
