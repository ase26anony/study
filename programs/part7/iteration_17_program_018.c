**Key features of this test file:**

1. **TYPE_UNDEFINED**: Forward declarations of `OpaqueStruct` and `OpaqueUnion` before their definitions.

2. **TYPE_SCALAR**: Multiple scalar typedefs (`scalar_int_t`, `scalar_float_t`, etc.).

3. **TYPE_STRING**: `string_type_t` as `const char*`.

4. **TYPE_STRUCT**: `SimpleStruct` as a plain struct.

5. **TYPE_USER_STRUCT**: `UserStruct` created via typedef struct.

6. **TYPE_UNION**: `DataUnion` and `ComplexUnion1`.

7. **TYPE_POINTER**: Various pointer types (`int_ptr_t`, `struct_ptr_t`, etc.).

8. **TYPE_ARRAY**: Fixed-size arrays (`int_array_10[10]`), incomplete arrays, and arrays of structs.

9. **TYPE_CALLBACK**: Multiple function pointer types with different signatures.

10. **TYPE_LANG_STRUCT**: `LangStruct` with GCC's `transaction_safe` attribute.

11. **Complex nesting**: 
    - `ComplexStruct1` contains an array of pointers to unions
    - `MasterStruct` contains almost every type category as members
    - Multiple levels of indirection and composition

12. **GTY markers**: All types that should be tracked by GC have appropriate `GTY` annotations.

13. **Global variables**: Ensure types are instantiated in the data segment.

14. **Function usage**: Functions that use the types prevent dead code elimination.

**To use this file for coverage:**

1. Place it in the `gcc/` directory of the GCC source tree.
2. Add it to the build by modifying the appropriate `Makefile.in` to include `gtype-test.o` in `gcc_objs`.
3. Build GCC with coverage instrumentation:
