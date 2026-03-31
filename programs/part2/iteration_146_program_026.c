## What's happening:

1. **`make_ref(std::move(obj))`**:
   - `std::move(obj)` returns an rvalue reference to `obj`
   - Template parameter `T` deduces to `U&&` (where `U` is the type of `obj`)
   - So `T&&` becomes `U&& &&` which collapses to `U&&` (rvalue reference)

2. **Inside `make_ref`**:
   - `std::forward<T>(t)` forwards `t` as an rvalue reference
   - `decltype(auto)` deduces the return type from the expression `std::forward<T>(t)`

3. **The problem**:
   - The function returns a reference to a temporary (the parameter `t`)
   - `t` is a function parameter that will be destroyed when `make_ref` returns
   - This creates a **dangling reference**

## Visualizing the issue:
