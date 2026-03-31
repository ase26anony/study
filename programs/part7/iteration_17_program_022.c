This test file provides:

1. **TYPE_UNDEFINED**: Forward declarations of `struct OpaqueStruct`, `union OpaqueUnion`, `class OpaqueClass`
2. **TYPE_SCALAR**: `int`, `float`, `double`, `char` global variables
3. **TYPE_STRING**: `char*` with string literal assignment
4. **TYPE_STRUCT**: `SimpleStruct` definition
5. **TYPE_USER_STRUCT**: `UserStruct` and `ComplexUserStruct` via typedef
6. **TYPE_UNION**: `DataUnion` and `MixedUnion`
7. **TYPE_POINTER**: Various pointer types including `int*`, `struct*`, `void*`, and double pointers
8. **TYPE_ARRAY**: Fixed arrays (`int_array[10]`), struct arrays, incomplete arrays (flexible array member), multi-dimensional arrays
9. **TYPE_CALLBACK**: Multiple function pointer types (`CompareFunc`, `TraverseFunc`, `FactoryFunc`)
10. **TYPE_LANG_STRUCT**: Structs with GCC attributes (`aligned`, `transaction_safe`)

The types are richly nested:
- `SuperNested` contains pointers to unions, arrays of struct pointers, callbacks, nested structs, unions with callbacks, and flexible arrays
- `MixedUnion` contains a struct with callbacks and language-specific structs
- Multiple levels of indirection and composition

To use this test:

1. Place `gtype-test.cc` in the `gcc/` directory of the GCC source tree
2. Build GCC with coverage instrumentation:
