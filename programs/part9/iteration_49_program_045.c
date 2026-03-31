This appears to be a switch statement from a C/C++ codebase that handles different types in some kind of type system or serialization system. Here's what each case seems to be doing:

## Type Categories:

1. **`TYPE_NONE`** - Unreachable case (likely an error state)
2. **`TYPE_UNDEFINED`** - Undefined/unknown types
3. **`TYPE_STRUCT`** - Regular C structs
4. **`TYPE_USER_STRUCT`** - User-defined structs (possibly with special handling)
5. **`TYPE_UNION`** - C unions
6. **`TYPE_POINTER`** - Pointer types
7. **`TYPE_ARRAY`** - Array types
8. **`TYPE_LANG_STRUCT`** - Language-specific structs (possibly for FFI/interop)
9. **`TYPE_SCALAR`** - Basic scalar types (int, float, etc.)
10. **`TYPE_STRING`** - String types
11. **`TYPE_CALLBACK`** - Callback/function pointer types

## Pattern:
Each type has a corresponding `write_state_*_type()` function that takes a `current` parameter (likely a context or type descriptor object).

## Likely Context:
This code is probably from:
- A serialization/deserialization system
- A code generator
- A debugger or introspection tool
- A compiler or language frontend
- A type system for FFI (Foreign Function Interface)

The `gcc_unreachable()` for `TYPE_NONE` suggests this is compiled with GCC and `TYPE_NONE` should never be encountered in normal execution.
