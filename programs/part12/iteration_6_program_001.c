This program creates:

1. **ARGUMENT_PACK_SELECT**: Through `Select<I, Ts...>` template alias using `std::tuple_element`
2. **DEFERRED_NOEXCEPT**: In `DeferredNoexceptTest::method()` and `deferred_noexcept_func()`
3. **TRAIT_EXPR**: Multiple uses of `std::is_same`, `std::is_base_of`, `std::is_constructible`, and `__is_constructible`
4. **LAMBDA_EXPR**: Various lambda expressions with different capture modes and a generic lambda
5. **STATIC_ASSERT**: Both regular and with location macro, in template and non-template contexts

**Recommended compilation command to trigger tree dumping:**
