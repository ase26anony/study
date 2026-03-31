**Key features that trigger the target code block:**

1. **Braced-init-list initialization**: Multiple arrays/vectors are initialized using `{...}` syntax, which calls `cp_build_vec_init_1`.

2. **Mixed lvalue and non-lvalue initializers**: Each initialization list contains:
   - An lvalue (`lval`)
   - Function calls (`get_value()`, `make_widget()`)
   - Cast expressions (`static_cast<int>(...)`)
   - Arithmetic expressions (`seed * seed + 1`)
   - Conditional operators (`(seed > 0) ? 10 : 20`)
   - Lambda calls

3. **User-defined types**: The `Widget` class engages more complex initialization paths.

4. **Prevention of optimization**:
   - `__attribute__((noipa))` prevents function inlining
   - `volatile` variables prevent dead code elimination
   - Command-line argument prevents compile-time evaluation
   - Observable output via `std::cout`

**Compilation commands for coverage analysis:**
