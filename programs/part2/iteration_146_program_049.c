## What's Happening

1. **`make_ref` function**:
   - Uses universal reference `T&& t` (forwarding reference)
   - Returns `decltype(auto)` to preserve the exact type
   - Uses `std::forward<T>(t)` for perfect forwarding

2. **Call site**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` is called

## The Problem

When you call `make_ref(std::move(obj))`:
- `T` is deduced as `Obj&&` (where `Obj` is the type of `obj`)
- The parameter becomes `Obj&& && t`, which collapses to `Obj&& t` (rvalue reference)
- `std::forward<Obj&&>(t)` returns `Obj&&`
- `decltype(auto)` deduces the return type as `Obj&&`

So `make_ref(std::move(obj))` returns an rvalue reference to `obj`.

## The Critical Issue

**`auto&& ref = make_ref(std::move(obj));` creates a dangling reference!**

Here's why:
- `make_ref(std::move(obj))` returns an rvalue reference to `obj`
- `auto&&` is a forwarding reference that binds to the rvalue reference
- But the reference is to `obj`, which might be destroyed or go out of scope
- `ref` now holds a dangling reference

## Example of the Danger
