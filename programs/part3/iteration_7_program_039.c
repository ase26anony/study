This appears to be a switch statement from a C/C++ codebase that handles different types in some kind of type system or serialization system. Here's what each case seems to be doing:

## Type Cases Breakdown:

1. **`TYPE_NONE`**: Marks this path as unreachable using `gcc_unreachable()` (likely an assertion for GCC compiler).

2. **`TYPE_UNDEFINED`**: Calls `write_state_undefined_type()` for undefined types.

3. **`TYPE_STRUCT`**: Handles regular struct types with `write_state_struct_type()`.

4. **`TYPE_USER_STRUCT`**: Handles user-defined struct types with `write_state_user_struct_type()`.

5. **`TYPE_UNION`**: Handles union types with `write_state_union_type()`.

6. **`TYPE_POINTER`**: Handles pointer types with `write_state_pointer_type()`.

7. **`TYPE_ARRAY`**: Handles array types with `write_state_array_type()`.

8. **`TYPE_LANG_STRUCT`**: Handles language-specific struct types with `write_state_lang_struct_type()`.

9. **`TYPE_SCALAR`**: Handles scalar types (integers, floats, etc.) with `write_state_scalar_type()`.

10. **`TYPE_STRING`**: Handles string types with `write_state_string_type()`.

11. **`TYPE_CALLBACK`**: Handles callback/function types with `write_state_callback_type()`.

## Context Clues:

This looks like it could be from:
- A **serialization/deserialization system** (writing type state)
- A **debugger or runtime type information system**
- A **compiler or language tool** handling type metadata
- A **reflection system** for C/C++

The pattern suggests this is part of a visitor or serializer that needs to handle different type kinds differently, likely for:
- Generating type metadata
- Serializing type information
- Debug information generation
- Runtime type checking

The `current` variable likely contains information about the type being processed, and each function specializes in writing/processing that specific type kind.
