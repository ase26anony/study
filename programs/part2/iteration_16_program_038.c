**Key elements that trigger the target code block:**

1. **Braced-init-list initialization**: Multiple arrays are initialized using `{...}` syntax, which calls `cp_build_vec_init_1`.

2. **Non-lvalue expressions in initializers**:
   - `get_value(seed)` - function call returning `int` (prvalue)
   - `static_cast<int>(get_double(seed))` - cast expression (prvalue)
   - `seed * 2 + 5` - arithmetic expression (prvalue)
   - `(seed > 20) ? 100 : 200` - conditional operator (prvalue)
   - `Widget(lval)` - temporary object (prvalue)
   - `make_widget(seed)` - function call returning `Widget` (prvalue)

3. **Mix of lvalue and non-lvalue**: Each initializer list contains both lvalues (`lval`) and non-lvalues.

4. **User-defined type**: `Widget` class with constructor and copy/move operations ensures full initialization logic is engaged.

5. **Anti-optimization measures**:
   - `__attribute__((noipa))` prevents interprocedural analysis
   - `volatile` members and variables prevent dead code elimination
   - Command-line argument prevents compile-time evaluation
   - Observable output via `std::cout`

**Compilation commands for coverage analysis:**
