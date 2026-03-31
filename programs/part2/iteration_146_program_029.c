## What's happening:

1. **`make_ref` function**:
   - Uses universal reference `T&& t` (forwarding reference)
   - Returns `decltype(auto)` to preserve the exact type
   - Uses `std::forward<T>(t)` for perfect forwarding

2. **Call site**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - When passed to `make_ref`, `T` deduces to `Obj&&` (where `Obj` is the type of `obj`)
   - So `T&&` becomes `Obj&& &&`, which collapses to `Obj&&` (rvalue reference)

## The Problem

The issue is that `ref` will be a **dangling reference** if `obj` is a local variable or temporary!
