## What's Happening

1. **`make_ref` function**:
   - Takes a forwarding reference `T&& t`
   - Uses `decltype(auto)` to preserve the exact type of the expression
   - Returns `std::forward<T>(t)` to forward the value category

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - When passed to `make_ref`, `T` deduces to `Obj` (not `Obj&`)
   - `std::forward<Obj>(t)` returns an rvalue reference `Obj&&`

3. **The problem**:
   - `make_ref(std::move(obj))` returns `Obj&&` (a dangling reference)
   - `auto&& ref` binds to this rvalue reference
   - **The reference `ref` becomes dangling** because the temporary from `std::move(obj)` doesn't extend the lifetime

## The Issue

The code creates a **dangling reference**. When you use `std::move(obj)` as an argument, it creates a temporary rvalue reference. The returned reference from `make_ref` points to this temporary, which doesn't get lifetime extension through `auto&& ref`.

## Lifetime Extension Rules

In C++, `auto&&` (or `T&&`) only extends the lifetime of temporaries in specific cases:
- When binding directly to a prvalue (temporary object)
- **NOT** when binding to an xvalue (rvalue reference returned from a function)

## Example of the Problem
