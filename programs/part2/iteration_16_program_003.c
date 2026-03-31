**Key features that trigger the target code block:**

1. **Braced-init-list initialization**: Multiple arrays are initialized using `{}`
2. **Mixed lvalue/non-lvalue initializers**: 
   - `lval` (lvalue)
   - `get_value(seed)` (non-lvalue function call)
   - `static_cast<int>(seed * 3.14)` (non-lvalue cast)
   - `(seed > 0) ? 42 : 24` (non-lvalue conditional)
   - `lval + 1` (non-lvalue arithmetic)
   - `Widget(lval)` (non-lvalue temporary)
   - `make_widget(seed)` (non-lvalue function call returning UDT)

3. **User-defined type**: `Widget` class with explicit constructors
4. **Prevention of constant folding**: 
   - `argc`-dependent values
   - `__attribute__((noipa))` functions
   - `volatile` variable
5. **Observable side effects**: Summation and output prevent dead code elimination

**Recommended compilation commands:**
