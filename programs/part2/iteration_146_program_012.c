### What happens step by step:

1. **`make_ref(std::move(obj))`**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `T` is deduced as `Obj&&` (where `Obj` is the type of `obj`)
   - The parameter `t` has type `Obj&& &&`, which collapses to `Obj&&` (rvalue reference)

2. **Inside `make_ref`**:
   - `std::forward<T>(t)` forwards `t` as an rvalue reference
   - Since `decltype(auto)` preserves the exact type, the return type is `Obj&&`

3. **`auto&& ref = ...`**:
   - `auto&&` is a forwarding reference (universal reference)
   - It binds to the returned `Obj&&`
   - `ref` becomes an rvalue reference to `obj`

## The Problem

The issue is that **`ref` is a dangling reference** if `obj` is a local variable or temporary. After the statement completes, `ref` refers to a potentially destroyed object.

### Example of the danger:
