**Key features that target the uncovered code:**

1. **Braced-init-list initialization**: Multiple arrays are initialized using `{...}` syntax, which triggers `cp_build_vec_init_1`.

2. **Mixed lvalue/non-lvalue expressions**:
   - `lval` (lvalue variable)
   - `get_value(seed)` (non-lvalue: function call)
   - `static_cast<int>(seed * 2.5)` (non-lvalue: cast expression)
   - `(seed > 0) ? seed : -seed` (non-lvalue: conditional expression)
   - `lval + 1` (non-lvalue: arithmetic expression)

3. **User-defined type**: `Widget` class with constructor and copy/move operations ensures the full initialization logic is engaged.

4. **Prevention of optimization**:
   - `__attribute__((noipa))` prevents interprocedural analysis/optimization
   - `volatile` members and variables prevent dead code elimination
   - Command-line argument prevents compile-time evaluation

5. **Multiple initialization contexts**:
   - C-style array
   - `std::array`
   - Array of user-defined type

**Compilation commands for coverage analysis:**
