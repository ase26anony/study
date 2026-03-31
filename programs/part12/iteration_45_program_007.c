This code appears to be part of a switch statement that handles different types in some kind of type system, likely for a compiler or code generator. Here's what each case does:

## Type Cases Breakdown:

1. **`TYPE_NONE`**: Uses `gcc_unreachable()` - a GCC compiler hint that this code path should never be reached.

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
- The `current` variable likely contains information about the current type being processed
- This is probably part of a type serialization or code generation system
- The `write_state_*` functions suggest this is writing type information to some output (file, buffer, etc.)
- Could be part of a compiler's intermediate representation or a debug info generator

## Missing Context:
The code snippet doesn't show:
- The full switch statement declaration
- What `current` is (likely a struct/class pointer)
- Where this output is going
- The broader purpose of this type writing system

This pattern is common in compilers, debug info generators, or serialization systems that need to handle different type representations.
