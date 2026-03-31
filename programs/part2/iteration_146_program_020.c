### What's happening:

1. **`make_ref` function**:
   - Takes a forwarding reference `T&& t`
   - Returns `decltype(auto)` - this preserves the exact value category of the expression
   - Uses `std::forward<T>(t)` to perfectly forward the argument

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` receives an rvalue
   - Returns an rvalue reference (due to `std::forward` with rvalue)

3. **The problem**:
   - `auto&& ref = ...` creates a forwarding reference
   - It binds to the returned rvalue reference
   - **But**: The returned rvalue reference is a dangling reference!

## The Issue

When you call `make_ref(std::move(obj))`:
- The function receives an rvalue reference to `obj`
- It returns an rvalue reference to the same `obj`
- This reference dangles immediately because the function parameter `t` is destroyed when the function returns

**This creates undefined behavior** - `ref` becomes a dangling reference.

## Visual Example
