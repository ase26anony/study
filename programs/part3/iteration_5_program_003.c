**Key features that address the requirements:**

1. **Constant bounds**: Uses `constexpr` functions and `const_index` template to generate compile-time constant indices.

2. **MEM_P vs non-MEM_P paths**:
   - `memory_target_ops()`: Direct array accesses trigger memory targets
   - `non_memory_target_ops()`: Computations that store results in registers

3. **Count control**:
   - `count = 2`: `char_arr[5..6]`
   - `count = 6`: `char_arr[10..15]` (small element type)
   - `count = 4`: `int_arr[0..3]` (larger element type)

4. **Volatile usage**: `volatile_wrapper` and volatile pointers prevent early constant folding.

5. **Conditional paths**: `mixed_operations()` uses `argc` to select different constant index pairs.

6. **Multiple element types**: Tests with `char`, `int`, and `long long` to explore different `TYPE_SIZE * count` products.

**Compilation recommendations:**
