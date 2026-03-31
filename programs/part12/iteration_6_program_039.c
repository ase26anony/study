This program creates all the required AST nodes:

1. **ARGUMENT_PACK_SELECT**: Through `Select<I, Ts...>` template alias using `std::tuple_element`
2. **DEFERRED_NOEXCEPT**: Through `noexcept(noexcept(T(...)))` expressions in template contexts
3. **TRAIT_EXPR**: Through `std::is_same`, `std::is_base_of`, `std::is_constructible`, and `__is_constructible`
4. **LAMBDA_EXPR**: Multiple lambdas with different capture modes, generic lambdas, and constexpr lambdas
5. **STATIC_ASSERT**: Both simple and complex static_asserts, including ones with location macros

To maximize coverage of the uncovered lines, compile with:
