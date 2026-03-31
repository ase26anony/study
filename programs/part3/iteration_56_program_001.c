## What's happening:

1. **`ExplicitClass e1(42);`** - ✓ Works fine
   - Direct initialization, calls `explicit ExplicitClass(int)`

2. **`ExplicitClass e2 = ExplicitClass(42);`** - ✗ **COMPILE ERROR**
   - Copy initialization requires implicit conversion
   - `ExplicitClass(42)` creates a temporary
   - Then tries to copy it using the copy constructor
   - But the copy constructor is `explicit`, so this fails

3. **`func(ExplicitClass(10));`** - ✓ Works fine
   - Creates temporary via direct initialization
   - Passed to function (copy/move may be elided by compiler)

## The actual compilation error:

For line 9: `ExplicitClass e2 = ExplicitClass(42);`

The compiler will complain that it cannot use the explicit copy constructor in a copy-initialization context. This is because `= ExplicitClass(42)` is copy-initialization, which requires the constructor to be non-explicit.

## To fix this:

If you want to allow copy-initialization, remove `explicit` from the copy constructor:
