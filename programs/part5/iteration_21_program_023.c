This program combines all the requested features:

1. **Explicit constructors/operators**: `ExplicitTest`, `TemplateExplicit`, `Container`
2. **Optional/variadic templates**: `std::optional`, variadic templates and functions
3. **Mutable members and bit-fields**: `ComplexBitfield`, `OuterWithBitfields`
4. **Picture strings**: `pascal_array` typedef, `pascal_func`
5. **Segment/TLS**: `thread_local` variables (conditional on x86_64)
6. **Small attribute**: `SmallStruct`, `PackedSmall`
7. **String length attributes**: `string_operations()` with `__builtin___memcpy_chk`
8. **Prototyped functions**: Both K&R and ANSI styles
9. **Template combinations**: Multiple template classes with explicit, mutable, optional
10. **Nested structures**: `UltimateTest` with deeply nested mutable and bit-fields

**Recommended compilation commands:**
