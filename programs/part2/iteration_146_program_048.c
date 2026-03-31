### What's happening:

1. **`make_ref` function**:
   - Takes a universal/forwarding reference `T&& t`
   - Uses `decltype(auto)` to preserve the exact type of the expression
   - Returns `std::forward<T>(t)` to forward the value category

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` receives an rvalue
   - Returns an rvalue reference to `obj`

### The Problem:

When you write:
