**Key features that trigger the target code block:**

1. **Braced-init-list initialization**: All containers (`arr`, `std_arr`, `vec`, `w_arr`, `lambda_arr`, `nested`) use `{...}` initialization.

2. **Mixed lvalue/non-lvalue initializers**: Each initialization list contains both lvalues (`lval`) and various non-lvalue expressions:
   - Function calls (`get_value()`, `make_widget()`)
   - Cast expressions (`static_cast<int>(...)`)
   - Arithmetic expressions (`seed * 2 + lval`)
   - Conditional operator (`(seed > 3) ? ... : ...`)
   - Lambda calls (`lambda()`)
   - Temporary objects (`Widget(...)`, `Inner(...)`)

3. **User-defined types**: `Widget` and `Inner` structs engage the full initialization logic with constructors.

4. **Prevention of optimization**:
   - `__attribute__((noipa))` prevents function inlining and interprocedural analysis
   - `volatile` variables prevent dead code elimination
   - `argv`-dependent values prevent compile-time evaluation
   - Output to `std::cout` creates observable side effects

5. **Multiple container types**: C-style arrays, `std::array`, `std::vector` all use braced-init-lists to ensure `cp_build_vec_init_1` is called.

**Compilation recommendations:**
