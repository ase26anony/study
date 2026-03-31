### What's happening:

1. **`make_ref` function**:
   - Takes a forwarding reference `T&& t`
   - Returns `decltype(auto)` to preserve value category
   - Uses `std::forward<T>(t)` for perfect forwarding

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` receives an rvalue
   - Returns an rvalue reference (since `std::forward` forwards rvalues as rvalues)
   - `auto&& ref` binds to this rvalue reference

## The Problem

**This creates a dangling reference!**

When you do:
