## What happens step by step:

1. **`make_ref(std::move(obj))`**:
   - `std::move(obj)` returns an rvalue reference to `obj`
   - Template parameter `T` deduces to `X&&` (where `X` is the type of `obj`)
   - So `T&&` becomes `X&& &&` which collapses to `X&&` (rvalue reference)

2. **Inside `make_ref`**:
   - `t` is of type `X&&` (rvalue reference)
   - `std::forward<T>(t)` forwards it as an rvalue reference

3. **Return type `decltype(auto)`**:
   - This deduces the exact type of the expression `std::forward<T>(t)`
   - Since `std::forward<X&&>` returns `X&&`, the return type is `X&&`

4. **`auto&& ref = ...`**:
   - `auto&&` is a forwarding reference (universal reference)
   - It binds to the returned `X&&`
   - Reference collapsing occurs: `X&& &&` → `X&&`

## The Problem

**`ref` becomes a dangling reference!**

Here's why:
- `std::move(obj)` creates an rvalue reference to `obj`
- This rvalue reference is passed to `make_ref`
- `make_ref` returns an rvalue reference to the same object
- But `auto&& ref` now holds an rvalue reference to `obj`
- If `obj` is a local variable that goes out of scope, `ref` becomes dangling

## Example of the danger:
