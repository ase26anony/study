## Key Features of This Test:

1. **TYPE_UNDEFINED**: Forward declarations of `OpaqueStruct` and `OpaqueUnion` at the top.

2. **TYPE_SCALAR**: Multiple typedefs for `int`, `float`, `double`, `char`, `bool`.

3. **TYPE_STRING**: `string_type` typedef and usage with string literal.

4. **TYPE_STRUCT**: `BasicStruct`, `ContainerStruct`, `CallbackManager`, etc.

5. **TYPE_USER_STRUCT**: `UserStruct`, `ComplexUserStruct` via typedef.

6. **TYPE_UNION**: `BasicUnion`, `MegaUnion`.

7. **TYPE_POINTER**: Various pointer types including `int_ptr`, `struct_ptr`, `void_func_ptr`.

8. **TYPE_ARRAY**: Fixed arrays (`int_array[10]`), struct arrays, incomplete arrays.

9. **TYPE_CALLBACK**: Multiple function pointer types including `callback_type`, `complex_callback`, `array_returning_func`.

10. **TYPE_LANG_STRUCT**: `LangStruct` with `transaction_safe` attribute and `PackedStruct` with `packed` attribute.

11. **Complex Nesting**: `ContainerStruct` contains members of almost all type categories.

12. **GTY Markers**: All relevant types marked with `GTY(())` for garbage collection.

13. **Actual Usage**: Functions that instantiate and use the types to prevent dead code elimination.

## Building and Testing:

To use this test file:

1. Place it in the `gcc/` directory of a GCC source tree.
2. Configure GCC with coverage instrumentation:
