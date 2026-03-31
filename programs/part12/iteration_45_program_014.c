This appears to be a `switch` statement handling different type categories in what looks like a compiler or code generation system. Here's an analysis:

## Structure
- A `switch` statement on some type classification
- Each case handles a specific type category with a corresponding function call
- `TYPE_NONE` is marked as unreachable (likely an error state)

## Type Categories

1. **TYPE_NONE** - Error/unreachable state
2. **TYPE_UNDEFINED** - Undefined types
3. **TYPE_STRUCT** - C-style structures
4. **TYPE_USER_STRUCT** - User-defined structures
5. **TYPE_UNION** - Union types
6. **TYPE_POINTER** - Pointer types
7. **TYPE_ARRAY** - Array types
8. **TYPE_LANG_STRUCT** - Language-specific structures
9. **TYPE_SCALAR** - Scalar types (integers, floats, etc.)
10. **TYPE_STRING** - String types
11. **TYPE_CALLBACK** - Callback/function pointer types

## Likely Context
This is probably from:
- A compiler's type system implementation
- A code generator/serializer for type information
- Part of a state machine for type processing
- Could be from GCC or a similar compiler infrastructure

## Function Pattern
All functions follow the pattern `write_state_*_type(current)`, suggesting this is part of a serialization or code generation phase where type information needs to be written/processed.

The `gcc_unreachable()` for `TYPE_NONE` suggests this is indeed from GCC or a GCC-compatible compiler infrastructure.
