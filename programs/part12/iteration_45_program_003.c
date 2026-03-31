This appears to be a switch statement from a C/C++ codebase that handles different data types for serialization or state writing. Here's an analysis:

## Code Purpose
This switch statement is part of a type serialization system that writes type information to some output (likely for persistence, debugging, or communication).

## Type Categories

1. **TYPE_NONE** - Invalid/placeholder type (unreachable)
2. **TYPE_UNDEFINED** - Uninitialized/unknown types
3. **TYPE_STRUCT** - Standard C structs
4. **TYPE_USER_STRUCT** - User-defined structs (possibly with custom serialization)
5. **TYPE_UNION** - C unions
6. **TYPE_POINTER** - Pointer types
7. **TYPE_ARRAY** - Array types
8. **TYPE_LANG_STRUCT** - Language-specific structs (possibly for FFI/interop)
9. **TYPE_SCALAR** - Basic scalar types (int, float, etc.)
10. **TYPE_STRING** - String types
11. **TYPE_CALLBACK** - Function pointer/callback types

## Key Observations:
- `gcc_unreachable()` for `TYPE_NONE` indicates this should never be reached in practice
- Each type has a dedicated write function
- `current` appears to be a context/state object passed to each function
- The pattern suggests a visitor-like approach for type serialization

## Possible Context:
This could be from:
- A compiler/interpreter's type system
- A serialization library
- A debugger or profiling tool
- A data persistence framework

The code is well-structured with clear separation of concerns for different type categories.
