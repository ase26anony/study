## The problem with explicit copy constructors:

When you mark a copy constructor as `explicit`, you prevent:
- Copy initialization: `T b = a;`
- Pass-by-value: `void f(T t)` called as `f(a);`
- Return-by-value: `T f() { return a; }`

## To fix this code:
