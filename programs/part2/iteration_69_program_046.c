This is a switch statement handling different type categories in what appears to be a code generator or serializer. Here's what each case does:

## Type Categories:

1. **TYPE_NONE** - Unreachable case (should never occur)
2. **TYPE_UNDEFINED** - Handles undefined/unknown types
3. **TYPE_STRUCT** - Standard C structures
4. **TYPE_USER_STRUCT** - User-defined structures
5. **TYPE_UNION** - C unions
6. **TYPE_POINTER** - Pointer types
7. **TYPE_ARRAY** - Array types
8. **TYPE_LANG_STRUCT** - Language-specific structures
9. **TYPE_SCALAR** - Basic scalar types (int, float, etc.)
10. **TYPE_STRING** - String types
11. **TYPE_CALLBACK** - Function pointer/callback types

## Key Observations:
- Each type category has a dedicated `write_state_*_type()` function
- The `current` variable likely contains type information/metadata
- This appears to be part of a state serialization or code generation system
- The `gcc_unreachable()` for TYPE_NONE ensures compiler optimization for the impossible case

## Possible Context:
This could be from:
- A compiler's intermediate representation (IR) serializer
- A debug information generator
- A reflection system for C/C++ code
- A code generator for bindings or serialization formats

The pattern suggests a system that needs to handle different type categories differently when outputting or processing type information.
