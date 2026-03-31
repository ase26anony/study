This is a switch statement that handles different type categories in what appears to be a code generator or serializer. Here's what each case does:

## Type Categories:

1. **TYPE_NONE**: Unreachable case (should never occur)
2. **TYPE_UNDEFINED**: Writes an undefined/unknown type
3. **TYPE_STRUCT**: Writes a C-style struct type
4. **TYPE_USER_STRUCT**: Writes a user-defined struct type
5. **TYPE_UNION**: Writes a union type
6. **TYPE_POINTER**: Writes a pointer type
7. **TYPE_ARRAY**: Writes an array type
8. **TYPE_LANG_STRUCT**: Writes a language-specific struct type
9. **TYPE_SCALAR**: Writes a scalar (basic) type (int, float, etc.)
10. **TYPE_STRING**: Writes a string type
11. **TYPE_CALLBACK**: Writes a callback/function pointer type

## Observations:
- This is likely part of a **type serialization system** or **code generator**
- The `current` variable likely contains type information/metadata
- Each `write_state_*_type` function handles serialization for a specific type category
- The code appears to be from a **compiler**, **debugger**, or **serialization library**
- `gcc_unreachable()` is a GCC extension that helps with optimization by indicating unreachable code

## Possible Contexts:
- **Compiler type system** (GCC/LLVM plugin)
- **Debug information generation** (DWARF, etc.)
- **Serialization for persistence** or **IPC**
- **Code generation for bindings** (FFI, language interop)

The pattern suggests a visitor-like approach to handling different type representations in a systematic way.
