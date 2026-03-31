**Key features that trigger the target code block:**

1. **Braced-init-list initialization**: Multiple arrays are initialized using `{...}` syntax, which calls `cp_build_vec_init_1`.

2. **Non-lvalue expressions in initializers**:
   - `get_value(seed)` - function call returning `int` (prvalue)
   - `static_cast<int>(get_double(seed))` - cast expression (prvalue)
   - `seed * 2` - arithmetic expression (prvalue)
   - `(seed > 0) ? 100 : 200` - conditional expression (prvalue)
   - `Widget(lval)` - temporary object (prvalue)
   - `make_widget(seed)` - function call returning `Widget` (prvalue)

3. **Mix of lvalue and non-lvalue**: Each initialization list contains both lvalues (`lval`) and non-lvalues.

4. **User-defined type**: `Widget` class with constructor and copy/move operations engages the full initialization logic.

5. **Prevention of optimization**:
   - `__attribute__((noipa))` prevents function inlining and interprocedural analysis
   - `volatile` members and variables prevent dead code elimination
   - Command-line argument prevents compile-time evaluation

6. **Observable side effects**: The final sum is computed and printed, ensuring all initialization code is executed.

**Compilation commands for coverage analysis:**
