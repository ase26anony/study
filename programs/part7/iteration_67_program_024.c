## Key Features Targeting Specific DWARF Attributes:

1. **`DW_AT_explicit`**: `ExplicitClass` has explicit constructors and conversion operators
2. **`DW_AT_is_optional`**: Uses `std::optional` and a C-style tagged union
3. **`DW_AT_location`/`DW_AT_segment`**: Variables with `register`, `volatile`, and custom section attributes
4. **`DW_AT_lower_bound`**: GNU C extension array with non-zero bounds (`-5...5`)
5. **`DW_AT_mutable`**: Class with `mutable` members and `const` members
6. **`DW_AT_string_length`**: Fixed-length string type `fixed_string[32]`
7. **`DW_AT_prototyped`**: Mix of old K&R and modern function prototypes
8. **`DW_AT_threads_scaled`**: `thread_local` and `__thread` variables
9. **`DW_AT_picture_string`**: Decimal picture structure (simulating COBOL-like types)
10. **Complex nesting**: Interdependent types with templates, namespaces, unions, and pointers

## Compilation Recommendations:

For maximum DWARF coverage:
