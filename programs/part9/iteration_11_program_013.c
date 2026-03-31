This test program covers all the required scenarios:

1. **Small element count (`count <= 2`)**: `test_small_count_memory()` creates single and two-element memory accesses.

2. **Larger constant-sized access**: `test_larger_constant_sized()` accesses 10 chars (80 bits) and 8 ints (256 bits), both fitting in unsigned HWI.

3. **Non-memory reference cases**: `test_non_memory_vector()` uses `__builtin_shufflevector` and vector compound literals that create `VEC_PERM_EXPR` and `CONSTRUCTOR` nodes instead of `MEM_REF`.

4. **Mixed scenarios**: Separate functions for different element types and access patterns.

5. **Control flow preservation**: Uses volatile variables in conditional expressions to prevent constant propagation from eliminating the code.

6. **Execution verification**: Computes and prints a checksum to ensure all code paths are executed.

To compile and test for coverage:
