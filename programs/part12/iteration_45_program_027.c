This appears to be a switch statement from a C/C++ codebase that handles different types for serialization or state writing. Here's an analysis:

## Key Observations:

1. **Type System**: The code implements a type system with various categories:
   - `TYPE_NONE`: Unreachable case (likely an error state)
   - `TYPE_UNDEFINED`: Undefined/void types
   - Composite types: `STRUCT`, `UNION`, `ARRAY`, `POINTER`
   - Specialized types: `USER_STRUCT`, `LANG_STRUCT` (language-specific structs)
   - Simple types: `SCALAR`, `STRING`
   - Function types: `CALLBACK`

2. **Serialization Pattern**: Each type has a corresponding `write_state_*_type()` function that serializes that specific type's state.

3. **Error Handling**: `gcc_unreachable()` for `TYPE_NONE` suggests this is compiled with GCC and uses compiler hints for optimization/error checking.

## Likely Context:

This is probably from:
- A compiler/interpreter's type system implementation
- A serialization/deserialization framework
- A debugger or state persistence system
- A code generator or intermediate representation writer

## Missing Cases:

The switch statement might be incomplete or there could be other types handled elsewhere. Common type system additions might include:
- `TYPE_ENUM`
- `TYPE_FUNCTION` (different from `TYPE_CALLBACK`)
- `TYPE_TYPEDEF`
- `TYPE_VOID`
- `TYPE_BITFIELD`

## Code Style:

The code follows a clean pattern with one case per line and consistent indentation. The use of `current` suggests it's processing a current type object/context.

Would you like me to:
1. Suggest the likely implementations of the `write_state_*_type` functions?
2. Show how a complete type system with these cases might be structured?
3. Explain how `gcc_unreachable()` works in this context?
