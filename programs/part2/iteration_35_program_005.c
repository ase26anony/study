## Key Features for Coverage:

1. **Multiple `static_assert` declarations** in various contexts:
   - In template class static member functions
   - In template class instance methods
   - Inside lambda expressions
   - In constrained function templates
   - Global scope static_assert

2. **Complex static_assert conditions** that generate rich AST nodes:
   - Uses `sizeof(T)` with template parameters
   - Uses `sizeof...(Args)` with parameter packs
   - Uses `std::is_integral_v<T>` trait expressions
   - Uses fold expressions `(std::is_same_v<Args, Args> && ...)`
   - Uses `constexpr` functions

3. **Other C++ features** from the uncovered context:
   - **Lambda expressions** with captures and `noexcept` specifications
   - **`requires` clauses** (C++20) for constraints
   - **`noexcept` expressions** with conditions
   - Template argument packs and variadic templates

4. **Explicit template instantiations** to ensure the code isn't optimized away:
   - `template class Checker<int, double, char>;`
   - `template struct ArrayChecker<42>;`

## Compilation Commands:

To trigger the tree dumping that will exercise the `STATIC_ASSERT` case in `cxx_print_xnode`:
