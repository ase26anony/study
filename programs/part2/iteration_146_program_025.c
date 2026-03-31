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

When you call `make_ref(std::move(obj))`:
- `T` deduces to the object type (e.g., `MyClass`)
- The function returns `std::forward<MyClass>(t)`, which is an rvalue reference
- `auto&& ref` binds to this rvalue reference

**The issue**: The returned rvalue reference is a **dangling reference** because `std::move(obj)` doesn't extend the lifetime of the temporary. The reference `ref` will refer to a potentially destroyed object.

## Lifetime Extension Issue

Unlike with prvalues (temporary objects), **rvalue references don't get lifetime extension** when bound to `auto&&`:
