This program implements multiple strategies to trigger `double_int` comparisons:

1. **Switch Statements**: The `process_large_switch` function uses a switch statement with large 64-bit case values that may require `double_int` representation during lowering.

2. **Binary Search**: The `binary_search_large` function performs binary search on an array of large constants, generating comparison operations.

3. **Array Bounds Checking**: `array_access_with_large_index` performs bounds checking against a large constant size.

4. **Loop Termination**: `loop_with_large_limit` has a loop with a large termination constant.

5. **Template Metaprogramming**: Multiple template instantiations with large non-type template parameters force compile-time comparisons.

6. **Builtin Overflow Checks**: Using `__builtin_add_overflow` with large values may trigger internal `double_int` comparisons.

7. **Constexpr Functions**: `compare_large_values` and template specializations force compile-time evaluation of comparisons.

**Compilation recommendations:**
