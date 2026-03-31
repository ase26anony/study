### What it does:
1. `make_ref` is a forwarding function template that takes a universal reference (`T&& t`)
2. It uses `decltype(auto)` to preserve the exact value category of the returned expression
3. `std::forward<T>(t)` forwards `t` with its original value category
4. When called with `std::move(obj)`, it receives an rvalue reference

### The Problem:
The issue is that `make_ref` returns a **reference to a temporary** when given an rvalue. Specifically:
- `std::move(obj)` converts `obj` to an rvalue reference
- `make_ref` receives this rvalue reference as `t`
- `std::forward<T>(t)` forwards it as an rvalue reference
- The returned rvalue reference **dangles** because it refers to a function parameter `t` that gets destroyed when the function returns

### Visualizing the Lifetime Issue:
