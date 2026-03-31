### What it does:
1. `make_ref` is a forwarding function template that takes a universal reference (`T&& t`)
2. It uses `decltype(auto)` to preserve the exact reference type of the expression
3. It forwards the argument using `std::forward<T>(t)`
4. When called with `std::move(obj)`, it receives an rvalue reference

### The Problem:
When `make_ref(std::move(obj))` is called:
- `T` is deduced as `Obj` (not `Obj&` or `Obj&&`)
- `std::forward<T>(t)` returns `T&&`, which is `Obj&&` (an rvalue reference)
- `decltype(auto)` deduces the return type as `Obj&&`
- `auto&& ref` becomes `Obj&& ref` (an rvalue reference)

**The issue**: `ref` is now an rvalue reference to a potentially moved-from object. If `obj` was moved from, accessing `ref` could lead to undefined behavior.

## Better Alternatives

### Option 1: Return by value (for temporaries)
