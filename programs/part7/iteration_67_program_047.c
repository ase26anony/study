## Key Features Targeting Specific DWARF Attributes:

1. **DW_AT_explicit**: `ExplicitClass` has explicit constructors and conversion operators
2. **DW_AT_is_optional**: `std::optional` members and optional pointers in `TaggedUnion`
3. **DW_AT_location/DW_AT_segment**: `SectionData` in custom section, `register` variable
4. **DW_AT_lower_bound**: GNU extension arrays with non-zero lower bounds (`-5...5`)
5. **DW_AT_mutable**: `mutable` members in `MutableClass` and `Container`
6. **DW_AT_string_length**: `FixedString` typedef for fixed-length strings
7. **DW_AT_prototyped**: Mix of prototyped and old-style K&R function declarations
8. **DW_AT_threads_scaled**: `__thread` and `thread_local` variables
9. **DW_AT_picture_string**: Attempted with packed structs and scalar storage order
10. **Complex nesting**: Deeply nested types in `DeepNesting` namespace

## Compilation Commands:
