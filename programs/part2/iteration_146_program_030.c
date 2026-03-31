## What's happening:

1. **`make_ref` function**:
   - Uses universal reference `T&& t`
   - Returns `decltype(auto)` to preserve value category
   - Uses `std::forward<T>(t)` for perfect forwarding

2. **The call**:
   - `make_ref(std::move(obj))` passes an rvalue reference
   - `T` deduces to `Obj&&` (where `Obj` is the type of `obj`)
   - Parameter `t` becomes `Obj&& &&` which collapses to `Obj&&`

3. **The return**:
   - `std::forward<Obj&&>(t)` returns `Obj&&`
   - `decltype(auto)` deduces return type as `Obj&&`
   - Function returns an rvalue reference

## The Problem

The issue is that `ref` becomes a **dangling reference**:
