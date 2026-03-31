## Key Features That Trigger the Target Code:

1. **Braced-init-list Initialization**: Multiple arrays are initialized using `{...}` syntax, which calls `cp_build_vec_init_1`.

2. **Non-lvalue Expressions**:
   - `get_value(seed)` - Function call returning `int` (prvalue)
   - `static_cast<int>(get_double(seed))` - Cast expression (prvalue)
   - `seed * seed + 1` - Arithmetic expression (prvalue)
   - `(seed > 5) ? 100 : 200` - Conditional operator (prvalue)
   - `Widget(lval)` - Temporary object (prvalue)
   - `make_widget(seed)` - Function call returning `Widget` (prvalue)

3. **Mix of Lvalue and Non-lvalue**: Each initialization list contains both lvalues (`lval`) and non-lvalues.

4. **User-defined Type**: `Widget` class with constructor and copy/move operations ensures the full initialization logic is engaged.

5. **Prevention of Optimization**:
   - `__attribute__((noipa))` prevents function inlining and interprocedural analysis
   - `volatile` members and variables prevent dead code elimination
   - Command-line argument dependency prevents compile-time evaluation

6. **Observable Side Effects**: Printing results ensures the initialization code isn't optimized away.

## Compilation Commands for Coverage Analysis:
