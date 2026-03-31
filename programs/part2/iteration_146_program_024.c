## What's Happening

1. **`make_ref` function**:
   - Takes a universal reference `T&& t`
   - Returns `decltype(auto)` to preserve value category
   - Uses `std::forward<T>(t)` for perfect forwarding

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` receives an rvalue
   - Returns an rvalue reference (xvalue)

3. **The problem**:
   - `auto&& ref = ...` creates a forwarding reference
   - It binds to the rvalue reference returned by `make_ref`
   - **But**: The returned rvalue reference becomes a dangling reference!

## The Issue

When you write:
