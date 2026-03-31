### What it does:
1. **`make_ref` function**: 
   - Takes a forwarding reference (`T&& t`)
   - Uses `decltype(auto)` to preserve the exact type of the expression
   - Returns `std::forward<T>(t)` to forward the value category

2. **Usage**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` forwards this rvalue reference
   - `auto&& ref` binds to the returned rvalue reference

### The Problem:
**This creates a dangling reference!**

When you write:
