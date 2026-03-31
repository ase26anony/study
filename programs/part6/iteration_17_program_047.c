**Key aspects of this program:**

1. **Multiple xvalue sources:**
   - `rval_ref_func()` returns `int&&` via `static_cast`
   - `std::move()` returns `int&&`
   - Direct `static_cast<int&&>(var)` creates an xvalue
   - `move_func()` wraps `std::move` in another function

2. **Prevents optimization:**
   - `__attribute__((noinline, noclone))` prevents inlining and cloning
   - `volatile` pointers prevent dead store elimination
   - `use_ptr()` function consumes results with a side effect

3. **Context for materialization:**
   - Direct initialization of `volatile int*` variables forces address computation
   - Multiple independent cases ensure at least one isn't optimized away

4. **Lvalue contrast:**
   - `&var` provides a baseline case that should take a different path

**Compilation commands to test coverage:**
