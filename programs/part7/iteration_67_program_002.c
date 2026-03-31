## Key Features Targeting Specific DWARF Attributes:

1. **`DW_AT_explicit`**: `ExplicitClass` has explicit constructor and conversion operator
2. **`DW_AT_is_optional`**: `std::optional<T>` usage and tagged union
3. **`DW_AT_location`/`DW_AT_segment`**: Variables with `__attribute__((section(...)))`
4. **`DW_AT_lower_bound`**: GNU extension array `int values[10][-5...5]`
5. **`DW_AT_mutable`**: `mutable` member in `ExplicitClass`
6. **`DW_AT_string_length*`**: Fixed string type `pstr[32]` and string containers
7. **`DW_AT_prototyped`**: Mix of K&R and ANSI function prototypes
8. **`DW_AT_threads_scaled`**: `__thread` variables with alignment
9. **`DW_AT_picture_string`**: `numeric_format.picture` struct (attempt for COBOL-like)
10. **Complex nesting**: Template recursion and namespace nesting

## Compilation Commands:
