### What's happening:

1. **`make_ref` function**:
   - Takes a forwarding reference `T&& t`
   - Returns `decltype(auto)` to preserve value category
   - Uses `std::forward<T>(t)` for perfect forwarding

2. **Usage**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` forwards this rvalue
   - `auto&& ref` binds to the returned value

## The Problem

When you pass `std::move(obj)` (an rvalue) to `make_ref`:
- `T` deduces to the type of `obj` (not a reference type)
- `T&&` becomes `obj_type&&` (rvalue reference)
- `std::forward<T>(t)` returns an rvalue reference
- `auto&& ref` binds to this rvalue reference

**The issue**: `ref` becomes a **dangling reference** if `obj` is a local variable or temporary!

## Example of the Danger
