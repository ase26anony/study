## What happens step by step:

1. **`std::move(obj)`** returns an rvalue reference to `obj`
2. **`make_ref(std::move(obj))`** is called with an rvalue
3. **Template deduction**: `T` is deduced as `Obj` (not `Obj&&` due to reference collapsing rules)
4. **Function parameter**: `T&&` becomes `Obj&&` (rvalue reference)
5. **Return type**: `decltype(auto)` deduces the exact type of the expression `std::forward<T>(t)`
   - `std::forward<Obj>(t)` returns `Obj&&` (rvalue reference)
   - So return type is `Obj&&`
6. **`auto&& ref`**: This is a forwarding reference that binds to the returned rvalue reference

## The Problem

The issue is that `ref` becomes a **dangling reference**! Here's why:

- `std::move(obj)` doesn't move anything by itself - it just casts to rvalue
- The actual move happens when the rvalue is used to initialize/assign something
- But in this case, we're just creating a reference to a temporary rvalue
- The reference `ref` refers to `obj`, but `obj` might have been moved from

## Example to illustrate:
