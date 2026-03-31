1. `std::move(obj)` returns `Obj&&` (rvalue reference)
2. Template argument deduction for `make_ref`:
   - `T&&` must match `Obj&&`
   - So `T` is deduced as `Obj` (not `Obj&` or `Obj&&`)
3. Inside `make_ref`:
   - Parameter `t` has type `T&&` which is `Obj&&`
   - `std::forward<T>(t)` returns `Obj&&`
   - `decltype(auto)` deduces the return type as `Obj&&`
4. `auto&& ref` binds to the returned `Obj&&`
   - `auto&&` is a forwarding reference that becomes `Obj&&` here

So `ref` is an rvalue reference to `obj`.

## The potential issue:

The problem is that `ref` is a **dangling reference** if `obj` is a local variable or temporary. Since `std::move` doesn't actually move anything (it just casts to rvalue), the reference is valid only as long as `obj` is alive.

## Example of the danger:
