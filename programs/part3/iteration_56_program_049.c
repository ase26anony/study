## The problem with explicit copy constructors:

When you mark a copy constructor as `explicit`, you prevent:
- Copy-initialization: `T b = a;`
- Pass-by-value function calls (unless you explicitly construct a temporary)
- Return-by-value (unless you explicitly construct the return value)

## To fix the code:

If you want to allow normal copy semantics, remove `explicit` from the copy constructor:
