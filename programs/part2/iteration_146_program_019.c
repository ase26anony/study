## What's Happening

1. **`make_ref` function**:
   - Takes a forwarding reference `T&& t`
   - Returns `decltype(auto)` to preserve the value category
   - Uses `std::forward<T>(t)` to forward the value category

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` receives an rvalue
   - Returns an rvalue reference (because of perfect forwarding)

3. **The problem**:
   - `auto&& ref` is a forwarding reference that can bind to rvalues
   - It successfully binds to the returned rvalue reference
   - **But**: The rvalue reference returned by `make_ref` is a dangling reference!

## The Dangling Reference Issue

When you call `make_ref(std::move(obj))`:
- The parameter `t` is an rvalue reference to `obj`
- `std::forward<T>(t)` returns an rvalue reference to `obj`
- This reference is returned from the function
- After the function returns, `ref` holds a reference to a destroyed temporary (the function parameter `t`)

## Visualizing the Problem
