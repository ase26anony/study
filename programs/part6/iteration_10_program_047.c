This program is designed to trigger all the specific DWARF attributes:

1. **DW_AT_explicit**: Multiple classes with `explicit` constructors and conversion operators
2. **DW_AT_is_optional**: Extensive use of `[[maybe_unused]]` and `std::optional`
3. **DW_AT_mutable**: Structs with `mutable` members, nested in unions and other structs
4. **DW_AT_lower_bound**: Fortran-style array descriptors with non-zero lower bounds
5. **DW_AT_ordering**: Explicit ordering specification (column-major/Fortran ordering)
6. **DW_AT_picture_string**: Custom `PictureString` class simulating COBOL picture clauses
7. **DW_AT_prototyped**: Full function prototypes throughout, with some C-compatible declarations
8. **DW_AT_small**: Small enums and structs with small integer types
9. **DW_AT_segment**: Simulated segment IDs in array descriptors
10. **DW_AT_string_length**: Explicit length strings (not null-terminated)
11. **DW_AT_string_length_bit_size**: Bit-fields for string length in bits
12. **DW_AT_string_length_byte_size**: Bit-fields for string length in bytes
13. **DW_AT_threads_scaled**: Thread-local variables with scaled indexing based on thread ID

The program performs actual computations and uses all constructs to ensure they appear in the debug information. The `main()` function orchestrates all tests and ensures execution flow through all code paths.

Compile with the recommended flags to maximize DWARF generation:
