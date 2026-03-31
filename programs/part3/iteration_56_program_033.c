## C++17 Guaranteed Copy Elision:

In C++17 and later, `ExplicitClass e2 = ExplicitClass(42);` works due to guaranteed copy elision - no actual copy/move occurs, so the explicit copy constructor isn't invoked.

## Debug Info Preservation:

The comment mentions "preserve explicit attribute in debug info" - this is a compiler implementation detail. The `explicit` keyword affects compile-time behavior, not runtime behavior, so debug info typically doesn't need to track it.

## Correct Usage Examples:
