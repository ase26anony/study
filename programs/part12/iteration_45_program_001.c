This appears to be a `switch` statement from a C/C++ codebase that handles different types in some kind of type system. Here's what each case seems to be doing:

## Type Categories:

1. **`TYPE_NONE`** - Uses `gcc_unreachable()` (likely a compiler hint that this case should never be reached)
2. **`TYPE_UNDEFINED`** - Handles undefined types
3. **`TYPE_STRUCT`** - Standard C struct types
4. **`TYPE_USER_STRUCT`** - User-defined struct types
5. **`TYPE_UNION`** - Union types
6. **`TYPE_POINTER`** - Pointer types
7. **`TYPE_ARRAY`** - Array types
8. **`TYPE_LANG_STRUCT`** - Language-specific struct types
9. **`TYPE_SCALAR`** - Scalar types (integers, floats, etc.)
10. **`TYPE_STRING`** - String types
11. **`TYPE_CALLBACK`** - Callback/function pointer types

## Observations:
- This looks like part of a type serialization or code generation system
- Each type has a corresponding `write_state_*_type()` function
- `current` is presumably a pointer to some type descriptor structure
- The code appears to be writing out type information in some format (possibly for debugging, serialization, or code generation)
- The use of `gcc_unreachable()` suggests this is compiled with GCC and the code is optimized

## Likely Context:
This could be from:
- A compiler/interpreter's type system
- A serialization library
- A debugger or analysis tool
- A code generator (like a binding generator for FFI)

The pattern suggests a visitor-like pattern where different type kinds are handled by different functions, all operating on the same `current` data structure.
