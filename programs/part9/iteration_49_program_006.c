This appears to be a switch statement from a C/C++ codebase that handles different types in some kind of type system. Here's what each case seems to be doing:

## Type Categories:

1. **TYPE_NONE** - Unreachable case (likely an error state)
2. **TYPE_UNDEFINED** - Handles undefined/unknown types
3. **TYPE_STRUCT** - Standard C struct types
4. **TYPE_USER_STRUCT** - User-defined struct types (possibly with special handling)
5. **TYPE_UNION** - C union types
6. **TYPE_POINTER** - Pointer types
7. **TYPE_ARRAY** - Array types
8. **TYPE_LANG_STRUCT** - Language-specific struct types (possibly for FFI or bindings)
9. **TYPE_SCALAR** - Basic scalar types (int, float, char, etc.)
10. **TYPE_STRING** - String types
11. **TYPE_CALLBACK** - Callback/function pointer types

## Context Clues:
- The functions `write_state_*_type()` suggest this is part of a serialization or code generation system
- The `current` parameter likely refers to a current type being processed
- This could be from:
  - A compiler/interpreter's type system
  - A serialization library
  - A code generator
  - A debugging or introspection tool
  - An FFI (Foreign Function Interface) system

## Notable Details:
- `gcc_unreachable()` is a GCC compiler hint indicating the TYPE_NONE case should never be reached
- The separation between TYPE_STRUCT and TYPE_USER_STRUCT suggests different handling for built-in vs user-defined structs
- TYPE_LANG_STRUCT might handle structs from other languages (like Python objects, Java classes, etc.)
- TYPE_CALLBACK is interesting - suggests support for function pointers or closures

This looks like it could be part of a language runtime, compiler backend, or serialization framework that needs to handle various type representations.
