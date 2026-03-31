**Key Coverage Elements:**

1. **Multiple `static_assert` declarations** with:
   - Dependent conditions using `sizeof(T)`
   - Parameter packs with `sizeof...(Args)`
   - `constexpr` function calls
   - Trait expressions (`std::is_integral_v<T>`)
   - Non-type template parameters

2. **Lambda expressions** in various contexts:
   - Capturing by reference `[&obj]`
   - With `noexcept` specifications
   - `constexpr` lambdas
   - Inside template methods

3. **C++20 features**:
   - `requires` clauses
   - Template constraints
   - `if constexpr` with requires expression

4. **Template structures** that ensure AST retention:
   - Primary template with parameter pack
   - Partial specialization
   - Explicit template instantiations
   - Non-type template parameters

5. **Execution flow**:
   - `main()` calls template methods
   - Explicit instantiations force code generation
   - Anonymous namespace prevents external linkage issues

**Compilation commands to trigger coverage:**
