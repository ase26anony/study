## The `explicit` keyword effects:

1. **`explicit ExplicitClass(int)`**: Prevents implicit conversions from `int` to `ExplicitClass`
2. **`explicit ExplicitClass(const ExplicitClass&)`**: Makes the copy constructor explicit, which affects copy-initialization

## To fix the compilation error:

You have two options:

**Option 1: Remove `explicit` from the copy constructor:**
