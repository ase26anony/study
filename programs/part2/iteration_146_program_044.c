## What's happening:

1. **`make_ref(std::move(obj))`**:
   - `std::move(obj)` returns an rvalue reference to `obj`
   - Template argument `T` deduces to `U&&` (where `U` is the type of `obj`)
   - So `T&&` becomes `U&& &&` which collapses to `U&&` (rvalue reference)

2. **Inside `make_ref`**:
   - `std::forward<T>(t)` forwards the rvalue reference
   - `decltype(auto)` deduces the return type as `U&&` (rvalue reference)

3. **`auto&& ref = ...`**:
   - `auto&&` is a forwarding reference (universal reference)
   - It binds to the rvalue reference returned by `make_ref`
   - `ref` becomes an rvalue reference to `obj`

## The Problem

The issue is that **`ref` is a dangling reference** if `obj` is a local variable or temporary!

### Example of the problem:
