**Key aspects that trigger the target code:**

1. **Braced-init-list initialization**: Multiple arrays are initialized using `{...}` syntax, which calls `cp_build_vec_init_1`.

2. **Non-lvalue expressions in initializers**:
   - `get_value(seed)` - function call returning `int` (prvalue)
   - `static_cast<int>(seed * 2.5)` - cast expression (prvalue)
   - `(seed > 0) ? 100 : 200` - conditional operator result (prvalue)
   - `lval + 5` - arithmetic expression (prvalue)
   - `Widget(lval)` - temporary object construction (prvalue)
   - `make_widget(seed)` - function call returning `Widget` (prvalue)

3. **Mix of lvalue and non-lvalue**: The first element of `c_array` is `lval` (an lvalue), while the others are non-lvalues.

4. **User-defined type**: `Widget` class with constructor and copy/move operations ensures the full initialization logic is engaged.

5. **Prevention of optimization**:
   - `__attribute__((noipa))` prevents interprocedural analysis/optimization
   - `volatile` members and variables prevent dead code elimination
   - Using `argc/argv` prevents compile-time constant folding
   - Output to `std::cout` creates observable side effects

**Compilation commands for coverage analysis:**
