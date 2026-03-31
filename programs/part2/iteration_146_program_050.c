## What's happening:

1. **`make_ref` function**:
   - Takes a forwarding reference `T&& t`
   - Uses `decltype(auto)` to preserve the exact type of the expression
   - Returns `std::forward<T>(t)` which forwards the value category

2. **Call site**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - When passed to `make_ref`, `T` deduces as `Obj&&` (where `Obj` is the type of `obj`)
   - Inside `make_ref`: `std::forward<Obj&&>(t)` returns `Obj&&`

3. **Reference collapsing**:
   - The return type with `decltype(auto)` becomes `Obj&&` (rvalue reference)
   - `auto&& ref` is a universal/forwarding reference
   - When initialized with `Obj&&`, reference collapsing occurs:
     - `auto&&` + `Obj&&` → `Obj&&`

## The Problem

**`ref` becomes a dangling rvalue reference!**

Since `std::move(obj)` creates a temporary rvalue reference, and `make_ref` returns that same rvalue reference, `ref` ends up referring to a moved-from object. This is dangerous because:

1. The object may have been moved from (its state is unspecified)
2. The object might be destroyed soon after if it's a temporary

## Example of the danger:
