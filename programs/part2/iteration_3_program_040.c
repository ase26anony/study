**Key features that target the uncovered code:**

1. **Multiple `static_assert` declarations** at different scopes (lines 10, 17, 22, 30, 49, 59, 65, 76, 82, 86, 114, 120, 124, 132)
2. **Macro usage** (`ASSERT_SIZE`, `ASSERT_ALIGN`) to test source location preservation
3. **Lambda expressions** to trigger the `LAMBDA_EXPR` case immediately before `STATIC_ASSERT` in the switch
4. **Template instantiations** that force evaluation of static assertions
5. **Mixed optimization levels** via `__attribute__((optimize("O0")))`
6. **C++20 concepts**, SFINAE, constexpr functions, and partial specializations
7. **Distinct source lines** for each static assertion to preserve location info

**Compilation command to trigger the coverage:**
