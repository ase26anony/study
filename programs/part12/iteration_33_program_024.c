This program implements all the requested patterns:

1. **Template Metaprogramming with Large Values**: The `LargeValue` template with `compare()` method forces compile-time comparisons of large constants.

2. **Switch Statements with Large Constants**: The `switch_large()` function uses large 64-bit constants as case labels, which GCC represents as `double_int` objects during switch lowering.

3. **Constant Expressions with Overflow Checks**: Uses `__builtin_add_overflow` and `__builtin_mul_overflow` with large constants that may trigger `double_int` comparisons internally.

4. **Multiple Comparison Contexts**:
   - Array indexing with bounds checking against large constants in `array_access_large()`
   - Loop termination conditions with large limits in `loop_large_limit()`
   - Binary search implementation with large constants in `binary_search_large()`

5. **Compile-time and Runtime Paths**: The `compile_time_comparisons()` function ensures compile-time evaluation, while runtime functions handle input-dependent values.

**Compilation and Testing:**
