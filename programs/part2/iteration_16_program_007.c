**Key features that trigger the target code block:**

1. **Braced-init-list initialization**: Multiple arrays are initialized using `{...}` syntax, which calls `cp_build_vec_init_1`.

2. **Non-lvalue expressions in initializers**:
   - `get_value(seed)` - function call returning int (prvalue)
   - `static_cast<int>(get_double(seed))` - cast expression (prvalue)
   - `seed * 2 + 5` - arithmetic expression (prvalue)
   - `Widget(lvalue_var)` - temporary object (prvalue)
   - `make_widget(seed)` - function call returning Widget (prvalue)
   - `(seed > 0) ? get_value(seed) : 0` - conditional expression (prvalue)
   - `lambda()` - lambda call (prvalue)

3. **Mixed lvalue/non-lvalue initializers**: Each array contains both lvalues (`lvalue_var`) and non-lvalues.

4. **User-defined type**: `Widget` class engages the full initialization logic with constructors.

5. **Prevention of optimization**:
   - `noipa` attributes prevent function inlining
   - `volatile` members and variables
   - Command-line argument dependency
   - Observable side effects (cout)

**Compilation commands for coverage analysis:**
