This appears to be a switch statement from a C/C++ codebase that handles different types in some kind of type system or serialization system. Here's what each case seems to be doing:

## Type Categories:

1. **TYPE_NONE** - Unreachable case (likely an error state)
2. **TYPE_UNDEFINED** - Undefined/unknown type
3. **TYPE_STRUCT** - Standard structure type
4. **TYPE_USER_STRUCT** - User-defined structure type
5. **TYPE_UNION** - Union type
6. **TYPE_POINTER** - Pointer type
7. **TYPE_ARRAY** - Array type
8. **TYPE_LANG_STRUCT** - Language-specific structure type
9. **TYPE_SCALAR** - Scalar/basic types (int, float, etc.)
10. **TYPE_STRING** - String type
11. **TYPE_CALLBACK** - Callback/function pointer type

## Pattern:
For each type, it calls a specific `write_state_*_type()` function, passing `current` (likely a pointer to the current type being processed).

## Likely Context:
This is probably from:
- A compiler/interpreter type system
- A serialization/deserialization system
- A debugger or introspection tool
- A code generator or documentation tool

The `gcc_unreachable()` for `TYPE_NONE` suggests this is compiled with GCC and `TYPE_NONE` should never be encountered in normal execution.
