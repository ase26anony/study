This is a switch statement handling different type cases in what appears to be a code generator or serializer. Here's what each case does:

## Type Cases Breakdown:

1. **`TYPE_NONE`**: Marks the code as unreachable using `gcc_unreachable()` (likely a compiler hint for optimization).

2. **`TYPE_UNDEFINED`**: Calls `write_state_undefined_type()` for undefined types.

3. **`TYPE_STRUCT`**: Handles regular C structs with `write_state_struct_type()`.

4. **`TYPE_USER_STRUCT`**: Handles user-defined structs with `write_state_user_struct_type()`.

5. **`TYPE_UNION`**: Handles C unions with `write_state_union_type()`.

6. **`TYPE_POINTER`**: Handles pointer types with `write_state_pointer_type()`.

7. **`TYPE_ARRAY`**: Handles array types with `write_state_array_type()`.

8. **`TYPE_LANG_STRUCT`**: Handles language-specific structs (possibly for FFI or bindings) with `write_state_lang_struct_type()`.

9. **`TYPE_SCALAR`**: Handles scalar types (integers, floats, etc.) with `write_state_scalar_type()`.

10. **`TYPE_STRING`**: Handles string types with `write_state_string_type()`.

11. **`TYPE_CALLBACK`**: Handles callback/function pointer types with `write_state_callback_type()`.

## Context Clues:
- The `current` parameter passed to each function suggests this is processing some AST or type representation
- The `write_state_*` naming convention indicates this is serializing or generating code for type information
- This could be part of:
  - A compiler's code generator
  - A serialization library for type information
  - A language binding generator (like SWIG or similar)
  - A debug information generator

The code appears well-organized with clear separation of concerns for different type categories.
