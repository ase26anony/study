## What's Actually Happening

1. **`ExplicitClass e1(42);`** - ✓ Valid
   - Direct initialization using the explicit converting constructor

2. **`ExplicitClass e2 = ExplicitClass(42);`** - ✗ **COMPILE ERROR**
   - This is copy initialization, which requires an implicit conversion
   - `ExplicitClass(42)` creates a temporary
   - Then tries to copy it using the copy constructor
   - But the copy constructor is `explicit`, so this fails

3. **`func(ExplicitClass(10));`** - ✓ Valid
   - Creates a temporary using direct initialization
   - Passes it to the function

## The Actual Problem

The line `ExplicitClass e2 = ExplicitClass(42);` **will not compile** because:
- The copy constructor is marked `explicit`
- Copy initialization requires an implicit copy constructor
- With an explicit copy constructor, you can only use direct initialization

## To Make It Work

If you want to preserve the explicit nature but allow this syntax, you could:
