### What it does:
1. `make_ref` is a forwarding function template that takes a universal reference (`T&& t`)
2. It uses `decltype(auto)` to preserve the exact value category of the returned expression
3. `std::forward<T>(t)` forwards `t` with its original value category
4. When called with `std::move(obj)`, it receives an rvalue reference

### The Problem:
**This creates a dangling reference!**

When you call `make_ref(std::move(obj))`:
- `std::move(obj)` converts `obj` to an rvalue reference
- This rvalue reference is passed to `make_ref`
- `make_ref` returns a reference to this temporary rvalue
- The returned reference becomes dangling immediately after the function returns

## Visual Example
