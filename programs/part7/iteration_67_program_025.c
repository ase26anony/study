## Key Features Targeting Specific DWARF Attributes:

1. **`DW_AT_explicit`**: `ExplicitClass` has explicit constructors and conversion operators
2. **`DW_AT_is_optional`**: Uses `std::optional<double>` and a tagged union with conditional members
3. **`DW_AT_location`/`DW_AT_segment`**: Variables with `__attribute__((section(".mysection")))` and `register` storage
4. **`DW_AT_lower_bound`**: GNU extension arrays with non-zero lower bounds `[-5 ... 5]`
5. **`DW_AT_string_length`**: `FixedString` typedef and `StringWithLength` struct
6. **`DW_AT_string_length_bit_size`**: Bit field in `StringWithLength::bit_length`
7. **`DW_AT_string_length_byte_size`**: Regular field in `StringWithLength::byte_length`
8. **`DW_AT_mutable`**: `mutable` members in `ExplicitClass` and `OrderedClass`
9. **`DW_AT_ordering`**: Mixed `mutable`, `const`, `volatile` with different access specifiers
10. **`DW_AT_picture_string`**: Attempt with packed `Currency` struct and GNU attributes
11. **`DW_AT_prototyped`**: Mix of K&R style (`var_args_func`) and modern prototypes
12. **`DW_AT_threads_scaled`**: `thread_local` variables and objects
13. **`DW_AT_small`**: Small enum with `: unsigned char` and bitfields

## Compilation Instructions:
