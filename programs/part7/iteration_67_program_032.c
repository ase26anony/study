This program specifically targets each uncovered DWARF attribute:

1. **`DW_AT_explicit`**: `ExplicitClass` with explicit constructor and conversion operator
2. **`DW_AT_is_optional`**: `std::optional<T>` and `TaggedUnion` with `is_present` flag
3. **`DW_AT_location`**: Register variable `reg_var`, volatile variables, and section attributes
4. **`DW_AT_lower_bound`**: GNU extension arrays `T array_member[10][-5...5]`
5. **`DW_AT_mutable`**: `mutable` members in classes
6. **`DW_AT_ordering`**: Arrays in classes that may require ordering information
7. **`DW_AT_picture_string`**: `#pragma` and `__attribute__((picture(...)))` hints
8. **`DW_AT_prototyped`**: Mix of old-style K&R and modern prototyped functions
9. **`DW_AT_segment`**: `__attribute__((section(...)))` on variables and structs
10. **`DW_AT_string_length`**: String structures with explicit length fields
11. **`DW_AT_string_length_bit_size`/`byte_size`**: Explicit bit/byte size fields
12. **`DW_AT_threads_scaled`**: `__thread` and `thread_local` variables with alignment

Compile with the recommended flags to maximize DWARF generation:
