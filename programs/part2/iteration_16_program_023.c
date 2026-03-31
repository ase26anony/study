## Key Features:

1. **Mixed Initialization in Braced-Init-Lists**: The program uses braced-init-lists for C-style arrays, `std::array`, `std::vector`, and arrays of user-defined types.

2. **Non-Lvalue Expressions**:
   - Function calls (`get_value()`, `make_widget()`)
   - Cast expressions (`static_cast<int>(...)`)
   - Conditional operator (`(seed > 0) ? 100 : 200`)
   - Arithmetic expressions (`lval + 5`)
   - Lambda calls (`lambda(seed)`)
   - Temporary objects (`Widget(lval)`)

3. **Mix of Lvalue and Non-Lvalue**: Each initialization list contains both lvalues (like `lval`) and non-lvalues.

4. **User-Defined Type**: The `Widget` struct has constructors and volatile members to prevent optimization.

5. **Prevention of Constant Folding**:
   - Uses `argc`/`argv` for seed value
   - `__attribute__((noipa))` on helper functions
   - `volatile` variables
   - Side effects in constructors

6. **Usage of Initialized Objects**: All arrays are read and their values summed to create observable side effects.

## Compilation Recommendations:
