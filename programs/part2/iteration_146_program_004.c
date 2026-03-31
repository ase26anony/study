### What happens step by step:

1. **`make_ref(std::move(obj))`**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `T` is deduced as `Obj&&` (where `Obj` is the type of `obj`)
   - `t` has type `Obj&& &&` which collapses to `Obj&&` (rvalue reference)

2. **Inside `make_ref`**:
   - `std::forward<T>(t)` forwards `t` as an rvalue reference
   - `decltype(auto)` deduces the return type as `Obj&&`

3. **`auto&& ref = ...`**:
   - `auto&&` is a forwarding reference (universal reference)
   - It binds to the returned `Obj&&`
   - `ref` becomes an rvalue reference to `obj`

## The Problem

**The issue is that `ref` is a dangling reference!**

When you do `std::move(obj)`, you're passing an rvalue reference to `obj`. The `make_ref` function returns this reference, and `auto&& ref` binds to it. However:

- `std::move(obj)` doesn't move anything by itself; it just casts to rvalue
- But `ref` is now an rvalue reference to `obj`
- If `obj` is destroyed or goes out of scope, `ref` becomes a dangling reference

## Example of the danger:
