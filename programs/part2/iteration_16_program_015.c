## Key Features:

1. **Braced-init-lists**: Uses `{}` initialization for C-style array, `std::array`, `std::vector`, and user-defined type array.

2. **Non-lvalue expressions**:
   - Function calls (`get_value(seed)`, `make_widget(seed)`)
   - Cast expressions (`static_cast<int>(d)`)
   - Arithmetic expressions (`seed * seed + 7`)
   - Conditional expressions (`(seed > 0) ? 100 : 200`)
   - Temporary objects (`Widget(lvalue_var)`)

3. **Mix of lvalue and non-lvalue**: Each initialization list contains both lvalues (like `lvalue_var`) and non-lvalues.

4. **User-defined type**: `Widget` struct with constructor and copy/move operations to engage full initialization logic.

5. **Prevents optimization**:
   - `__attribute__((noipa))` prevents inlining and interprocedural analysis
   - `volatile` variables prevent dead code elimination
   - Command-line argument prevents compile-time evaluation
   - Final output ensures side effects

## Compilation Options:

To maximize coverage of the target code block:
