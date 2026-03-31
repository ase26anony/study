This test file provides comprehensive coverage of all type categories:

1. **TYPE_UNDEFINED**: `OpaqueStruct` and `OpaqueUnion` forward declarations
2. **TYPE_SCALAR**: `int`, `float`, `double`, `char`, `bool`, enums, bitfields
3. **TYPE_STRING**: `const char*` with string literal assignment
4. **TYPE_STRUCT**: `BasicStruct`, `LangStruct`, `NestedStruct`, etc.
5. **TYPE_USER_STRUCT**: `UserStruct`, `ComplexUserStruct` via typedef
6. **TYPE_UNION**: `DataUnion`, `MixedUnion`, anonymous unions
7. **TYPE_POINTER**: Various pointer types including self-referential pointers
8. **TYPE_ARRAY**: Fixed-size, multi-dimensional, and incomplete arrays
9. **TYPE_CALLBACK**: Function pointers with various signatures
10. **TYPE_LANG_STRUCT**: Structs with GCC attributes like `transaction_safe` and `vector_size`

The file uses proper GCC conventions:
- Includes necessary headers (`config.h`, `system.h`, `coretypes.h`)
- Uses `GTY()` markers for garbage-collected types
- Uses `GTY((skip))` for pointers that shouldn't be traced
- Follows GCC naming conventions
- Includes actual code that uses the types to prevent dead code elimination

To use this file for coverage testing:

1. Place it in the `gcc/` directory of a GCC source tree
2. Configure GCC with coverage flags:
