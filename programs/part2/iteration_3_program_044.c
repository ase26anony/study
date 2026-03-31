**Key features that target the uncovered code:**

1. **Multiple `static_assert` declarations** at different scopes (lines 10, 17, 28, 43, 50, 59, 70, 77, 85, 90, 95, 102, 105, 120, 142, 150)

2. **Macro-generated `static_assert`s** (`ASSERT_SIZE`, `ASSERT_ALIGN`) that preserve source location information through macro expansion

3. **Template-dependent `static_assert`s** in `TypeChecker`, `Factorial`, and `PackChecker` that require template instantiation

4. **Lambda expressions** (lines with `lambda_with_assert` and `local_lambda`) to trigger `LAMBDA_EXPR` case before `STATIC_ASSERT`

5. **Mixed C++20 features** (concepts, `if constexpr`) to stress the front-end

6. **Function with optimization attribute** (`__attribute__((optimize("O0")))`) to potentially trigger different compilation paths

7. **Complex template metaprogramming** with recursive templates and SFINAE

**Compilation command to maximize coverage:**
