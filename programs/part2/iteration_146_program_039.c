## What's Happening

1. **`make_ref` function**:
   - Takes a forwarding reference `T&& t`
   - Uses `decltype(auto)` to preserve the exact type of the expression
   - Returns `std::forward<T>(t)` to forward the value category

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - When passed to `make_ref`, `T` deduces to the object type (not reference)
   - The parameter `t` becomes an rvalue reference to the object type

## The Problem

The issue is that **`ref` will be a dangling reference** if `obj` is a local variable or temporary!

Here's why:
- `std::forward<T>(t)` returns an rvalue reference to `t`
- `t` itself is a reference parameter (rvalue reference to `obj`)
- When the function returns, `t` (being a reference) doesn't extend the lifetime of what it refers to
- `ref` ends up as a dangling reference to `obj`

## Example of the Problem
