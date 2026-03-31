**Key features that trigger the target code block:**

1. **Braced-init-list initialization**: Multiple containers are initialized using `{...}` syntax, which calls `cp_build_vec_init_1`.

2. **Mixed lvalue and non-lvalue expressions**: Each initialization list contains:
   - `lval` (an lvalue variable)
   - `get_value(seed)` (non-lvalue function call)
   - Cast expressions like `static_cast<int>(get_double(seed))`
   - Conditional expressions `(seed > 20) ? 100 : 200`
   - Arithmetic expressions `seed * 2 + lval`
   - Lambda calls `lambda()`

3. **User-defined type**: `Widget` class with constructors ensures complex initialization paths are taken.

4. **Prevention of optimization**:
   - `__attribute__((noipa))` prevents function inlining and interprocedural analysis
   - `volatile` variables prevent dead code elimination
   - Command-line input prevents compile-time evaluation
   - Observable output via `std::cout`

**Compilation commands for coverage analysis:**
