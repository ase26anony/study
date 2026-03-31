## What's Happening

1. **`make_ref` function**:
   - Takes a forwarding reference `T&& t`
   - Returns `decltype(auto)` which deduces the exact type of the expression `std::forward<T>(t)`
   - Uses `std::forward` to preserve the value category (lvalue/rvalue-ness)

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` receives an rvalue
   - `T` is deduced as the type of `obj` (not a reference type)
   - `std::forward<T>(t)` returns an rvalue reference

## The Problem

When `make_ref` is called with an rvalue:
- `decltype(auto)` deduces to `T&&` (an rvalue reference)
- The function returns an rvalue reference to a temporary (the parameter `t`)
- This creates a **dangling reference** because `t` is a function parameter that gets destroyed when the function returns

## Visualizing the Issue
