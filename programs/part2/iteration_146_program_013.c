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
When `make_ref` is called with `std::move(obj)`:
- `T` is deduced as `Obj&&` (where `Obj` is the type of `obj`)
- The function returns `std::forward<Obj&&>(std::move(obj))` → an rvalue reference
- `auto&& ref` binds to this rvalue reference

**The issue**: `ref` is a dangling reference! It refers to the result of `std::move(obj)`, which is an xvalue (expiring value). If `obj` is destroyed or goes out of scope, `ref` becomes a dangling reference.

## Example of the Problem
