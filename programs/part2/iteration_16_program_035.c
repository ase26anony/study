**Key features that trigger the target code block:**

1. **Braced-init-lists**: Multiple initializations using `{...}` syntax for arrays, `std::array`, `std::vector`, and C-style arrays.

2. **Non-lvalue expressions**:
   - Function calls (`get_value()`, `create_widget()`, `get_double()`)
   - Cast expressions (`static_cast<int>(...)`)
   - Conditional operators (`(seed > 0) ? 100 : 200`)
   - Arithmetic expressions (`seed + lval * 2`)
   - Lambda calls (`lambda(seed)`)
   - Temporary objects (`Widget(lval)`, `Widget{seed * 2}`)

3. **Mix of lvalue and non-lvalue**: Each initialization list contains both lvalues (like `lval`) and non-lvalues.

4. **User-defined type**: `Widget` class with constructors and copy/move operations to engage full initialization logic.

5. **Prevention of optimization**:
   - `__attribute__((noipa))` on helper functions
   - `volatile` variables
   - Using `argc`/`argv` for seed value
   - Side effects in constructors
   - Output to `std::cout`

6. **Multiple contexts**: The non-lvalue initializers appear in various contexts (C-arrays, `std::array`, `std::vector`, single-type and user-type arrays).

**Compilation recommendations:**
