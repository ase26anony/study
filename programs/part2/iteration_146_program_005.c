## What's Happening

1. **`make_ref` function**:
   - Takes a universal/forwarding reference `T&& t`
   - Uses `decltype(auto)` to preserve the exact type of the expression
   - Returns `std::forward<T>(t)` to forward the value category

2. **The call**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` receives an rvalue
   - Returns an rvalue reference to `obj`

3. **The problem**:
   - `auto&& ref = make_ref(std::move(obj));` creates a **dangling reference**
   - `ref` is an rvalue reference to `obj`, but `obj` may have been moved from
   - The reference is bound to a temporary result of `std::move(obj)`

## The Issue

After `std::move(obj)`:
- `obj` is in a valid but unspecified state (if it's a move-constructible type)
- The reference `ref` points to `obj`, but `obj` may no longer contain meaningful data
- This is especially dangerous if `obj` is a local variable that might go out of scope

## Better Alternatives

### Option 1: Return by value (for temporaries)
