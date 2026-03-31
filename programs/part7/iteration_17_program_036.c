**Key features that ensure coverage of all type categories:**

1. **TYPE_UNDEFINED**: Forward declarations of `struct OpaqueStruct` and `union OpaqueUnion`
2. **TYPE_SCALAR**: `int`, `float`, `double`, `char`, `enum Color`
3. **TYPE_STRING**: `global_string` initialized with string literal
4. **TYPE_STRUCT**: `struct SimpleStruct`, `struct ComplexStruct`
5. **TYPE_USER_STRUCT**: `typedef struct { ... } Point`, `typedef struct { ... } PointList`
6. **TYPE_UNION**: `union DataUnion`, `union CallbackUnion`
7. **TYPE_POINTER**: Various pointer declarations including function pointers
8. **TYPE_ARRAY**: Fixed-size arrays, incomplete arrays, multi-dimensional arrays
9. **TYPE_CALLBACK**: Function pointer typedefs and members
10. **TYPE_LANG_STRUCT**: Structs with GCC attributes (`aligned`, `packed`, `transaction_safe`)

**To use this file for coverage testing:**

1. Place `gtype-test.cc` in the `gcc/` directory of the GCC source tree
2. Add it to the build by modifying `gcc/Makefile.in` or building manually
3. Configure GCC with coverage flags:
