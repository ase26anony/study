### What's happening:

1. **`make_ref` function template**:
   - Takes a forwarding reference `T&& t`
   - Returns `decltype(auto)` - this deduces the exact type of the expression `std::forward<T>(t)`
   - Uses `std::forward` to preserve the value category (lvalue/rvalue-ness)

2. **Call site**:
   - `std::move(obj)` converts `obj` to an rvalue reference
   - `make_ref(std::move(obj))` is called with an rvalue

### The Problem:

When `make_ref` is called with an rvalue (like `std::move(obj)`):
- `T` deduces to the object type (not reference type)
- `std::forward<T>(t)` returns an rvalue reference
- `decltype(auto)` deduces to an rvalue reference type
- The function returns an rvalue reference to a **temporary** (the parameter `t`)

This creates a **dangling reference** because:
- The parameter `t` is a local variable of the function
- When the function returns, `t` is destroyed
- `ref` ends up holding a reference to a destroyed object

## The Fix

To make this safe, you need to ensure the function doesn't return a reference to its parameter. Here are two correct approaches:

### Option 1: Return by value (for rvalues)
