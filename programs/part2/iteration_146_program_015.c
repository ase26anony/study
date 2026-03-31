## What's Happening

1. **`make_ref` function**:
   - Takes a forwarding reference `T&& t`
   - Returns `decltype(auto)` to preserve the value category
   - Uses `std::forward<T>(t)` for perfect forwarding

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` receives an rvalue reference
   - Returns an rvalue reference (due to perfect forwarding)
   - `auto&& ref` binds to this rvalue reference

## The Problem

**This creates a dangling reference!**

When you write:
