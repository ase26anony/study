## Key Design Elements:

1. **Multiple Braced-Init-Lists**: The program uses four different braced-init-lists for:
   - C-style array
   - `std::array`
   - `std::vector`
   - Array of user-defined `Widget` type

2. **Non-Lvalue Expressions**: Each list contains:
   - Function calls (`get_value()`, `make_widget()`)
   - Cast expressions (`static_cast<int>(...)`)
   - Conditional operator (`(seed > 50) ? 99 : -99`)
   - Arithmetic expressions (`seed * seed % 100`)
   - Temporary objects (`Widget(seed)`)

3. **Mixed Lvalue/Non-Lvalue**: Each list mixes lvalues (`lvalue_var`) with non-lvalues.

4. **User-Defined Type**: The `Widget` class has constructors and volatile members to prevent optimization.

5. **Anti-Optimization Measures**:
   - `__attribute__((noipa))` on helper functions
   - `volatile` variables
   - Command-line input for seed
   - Observable output

## Compilation Recommendations:
