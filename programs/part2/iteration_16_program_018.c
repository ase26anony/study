## Key Design Elements:

1. **Braced-init-list Initialization**: Uses `int arr[4] = {...}`, `std::array<int, 3> = {...}`, and `Widget widgets[3] = {...}` - all using braced-init-lists.

2. **Non-lvalue Expressions**:
   - `get_value(seed)` - function call returning `int` (prvalue)
   - `static_cast<int>(seed * 2.5)` - cast expression (prvalue)
   - `(seed > 5) ? 100 : 200` - conditional operator result (prvalue)
   - `make_widget(seed)` - function call returning `Widget` (prvalue)
   - `Widget(lval)` - temporary object (prvalue)

3. **Mix of Lvalue and Non-lvalue**: Each initialization list contains both lvalues (like `lval`) and non-lvalues.

4. **User-defined Type**: `Widget` struct with constructor and copy/move operations to engage full initialization logic.

5. **Prevention of Optimization**:
   - `__attribute__((noipa))` prevents interprocedural analysis and inlining
   - `volatile` members and variables prevent dead code elimination
   - Using `argc/argv` prevents compile-time evaluation
   - Output to `std::cout` creates observable side effects

6. **Execution Flow**: The program takes a seed from command line, creates various arrays with mixed initializers, computes a sum, and outputs it.

## Compilation Commands:
