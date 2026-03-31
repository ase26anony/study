## Key Design Elements:

1. **DW_AT_explicit**: `ExplicitClass` has explicit constructors and conversion operators
2. **DW_AT_is_optional**: `OptionalWrapper` uses `std::optional` and has conditional presence
3. **DW_AT_location/DW_AT_segment**: `section_var` uses section attribute, `reg_var` uses register
4. **DW_AT_lower_bound**: GNU extension `[-5..5]` array bounds (if supported)
5. **DW_AT_string_length***: `FixedString` and `WideString` typedefs
6. **DW_AT_mutable**: `mutable` members in `ExplicitClass` and `RecursiveTemplate`
7. **DW_AT_picture_string**: Attempt with GNU `__attribute__((picture(...)))`
8. **DW_AT_prototyped**: Mix of K&R (`old_style_func`) and modern prototypes
9. **DW_AT_threads_scaled**: `thread_local` variables with complex types
10. **Complex nesting**: `MasterStruct` contains multiple complex types, templates in namespace

## Compilation Commands:
