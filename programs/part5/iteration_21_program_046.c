This program combines multiple C++ features and GCC extensions to trigger the specific DWARF attributes:

1. **`DW_AT_explicit`**: Multiple explicit constructors and conversion operators
2. **`DW_AT_is_optional`**: `std::optional` parameters and template parameter packs
3. **`DW_AT_mutable`**: Mutable members in classes and templates
4. **`DW_AT_ordering`**: Complex bit-field layouts
5. **`DW_AT_picture_string`**: Pascal attribute and Fortran-style string simulation
6. **`DW_AT_lower_bound`**: Array processing with explicit bounds
7. **`DW_AT_segment`**: Segment register usage (conditionally compiled)
8. **`DW_AT_threads_scaled`**: Thread-local storage
9. **`DW_AT_small`**: Structs with `small` attribute
10. **`DW_AT_string_length*`**: String operations with explicit length
11. **`DW_AT_prototyped`**: Mix of K&R and modern function declarations
12. **`DW_AT_location`**: Functions returning pointers that may have complex locations

**Recommended compilation commands:**
