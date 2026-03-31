### What's happening:

1. **`make_ref` function**:
   - Takes a universal/forwarding reference `T&& t`
   - Returns `decltype(auto)` to preserve value category
   - Uses `std::forward<T>(t)` to perfectly forward the argument

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` receives an rvalue
   - Returns an rvalue reference (xvalue)

3. **`auto&& ref`**:
   - `auto&&` is a forwarding reference (universal reference)
   - It can bind to both lvalues and rvalues

## The Problem

**The issue is that `ref` becomes a dangling reference!**

When you do:
