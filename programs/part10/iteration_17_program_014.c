This test program includes:

1. **DEFERRED_NOEXCEPT nodes**: Through `noexcept(noexcept(T()))` in function templates and variable templates.

2. **TRAIT_EXPR nodes**: Through extensive use of `__is_same`, `__is_base_of`, `__is_constructible` in various contexts (static asserts, if constexpr, template arguments).

3. **LAMBDA_EXPR nodes**: Multiple lambda expressions with different capture modes ([=], [&], specific captures), generic lambdas, nested lambdas, and lambdas in algorithms.

4. **STATIC_ASSERT nodes**: Static asserts at global scope, namespace scope, class scope, and with dependent expressions in templates.

5. **Complex template combinations**: Templates that combine traits, noexcept, lambdas, and static asserts together.

6. **Forced instantiation**: Using `__attribute__((used))` and explicit template instantiations to ensure the compiler generates the internal tree nodes.

To compile and generate the tree dumps:
