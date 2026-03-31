## What's Happening

1. **`make_ref` function**:
   - Takes a forwarding reference `T&& t`
   - Uses `decltype(auto)` to preserve the exact type of the expression
   - Returns `std::forward<T>(t)` to forward the value category

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - When passed to `make_ref`, `T` deduces to the object type (not reference)
   - `t` becomes an rvalue reference to the object type

## The Problem

When you call `make_ref(std::move(obj))`:
- `T` deduces to the object type (e.g., `MyClass`)
- `t` has type `MyClass&&` (rvalue reference)
- `std::forward<T>(t)` returns `MyClass&&`
- `decltype(auto)` preserves this as `MyClass&&`

So `ref` gets type `auto&&` which deduces to `MyClass&&` (an rvalue reference).

**The issue**: You now have an rvalue reference bound to a temporary-like object, but the original object (`obj`) might still exist. This creates a **dangling reference** if `obj` is destroyed before `ref` is used.

## Example of the Danger
