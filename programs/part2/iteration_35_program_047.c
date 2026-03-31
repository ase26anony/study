**Key features that trigger the uncovered code paths:**

1. **Multiple `static_assert` declarations** with:
   - Non-trivial conditions using `sizeof`, `sizeof...`, and type traits
   - Different messages and some without messages (C++17 style)
   - Source locations preserved in template instantiations

2. **Lambda expressions** (`LAMBDA_EXPR` nodes):
   - Lambdas with captures and variadic parameter packs
   - Used within template methods

3. **`requires` clauses** (C++20):
   - Constrains the primary `Checker` template

4. **Variadic templates and pack expansions**:
   - `Checker<T, Args...>` with `sizeof...(Args)`
   - `PackSelector` with potential `ARGUMENT_PACK_SELECT` nodes

5. **`noexcept` expressions**:
   - `noexcept(noexcept(u + u))` may generate `DEFERRED_NOEXCEPT` nodes

6. **Trait expressions**:
   - `__is_same` usage in `is_same_size()` for `TRAIT_EXPR` nodes

7. **Explicit template instantiations**:
   - Forces AST generation even without optimization
   - Prevents code elimination

**Compilation commands to trigger the coverage:**
