## Key Features:

1. **Braced-init-lists**: Multiple arrays initialized with `{...}` syntax
2. **Non-lvalue expressions**:
   - Function calls (`get_value()`, `create_widget()`)
   - Cast expressions (`static_cast<int>(...)`)
   - Conditional operators (`(seed > 20) ? 100 : 200`)
   - Arithmetic expressions (`seed + lvalue_var`)
   - Lambda invocations (`lambda()`)
   - Constructor calls creating temporaries

3. **Mixed lvalue/non-lvalue**: Each initialization list contains both lvalues (like `lvalue_var`) and non-lvalues

4. **User-defined type**: `Widget` class with constructors and copy/move operations

5. **Usage prevents dead code**: All values are summed and printed, with volatile writes

6. **Input-dependent**: Uses `argc`/`argv` to prevent compile-time evaluation

## Compilation Recommendations:
