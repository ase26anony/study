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

When you do `std::move(obj)`, you're passing a temporary rvalue reference to `make_ref`. The function returns this reference, and `auto&& ref` binds to it. However:

- If `obj` is a local variable, `std::move(obj)` doesn't extend its lifetime
- `ref` now refers to `obj`, but there's no guarantee `obj` is still alive when `ref` is used
- This is a classic dangling reference scenario

## Example of the Danger
