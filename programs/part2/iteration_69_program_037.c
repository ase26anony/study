This is a switch statement handling different type categories in what appears to be a compiler or code generation system. Here's what each case does:

## Type Categories:

1. **TYPE_NONE**: Unreachable case - indicates an error state
2. **TYPE_UNDEFINED**: Handles undefined types
3. **TYPE_STRUCT**: Handles C-style struct types
4. **TYPE_USER_STRUCT**: Handles user-defined struct types
5. **TYPE_UNION**: Handles union types
6. **TYPE_POINTER**: Handles pointer types
7. **TYPE_ARRAY**: Handles array types
8. **TYPE_LANG_STRUCT**: Handles language-specific struct types
9. **TYPE_SCALAR**: Handles scalar types (integers, floats, etc.)
10. **TYPE_STRING**: Handles string types
11. **TYPE_CALLBACK**: Handles callback/function pointer types

## Key Observations:
- Each type category has a dedicated `write_state_*_type()` function
- `current` appears to be a context/state object passed to each function
- `gcc_unreachable()` is a GCC extension that tells the compiler this code path should never be reached
- The code follows a consistent pattern for type handling in a compiler backend

This looks like part of a type serialization or code generation system where different type representations need different handling logic.
