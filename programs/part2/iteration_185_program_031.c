This program is designed to trigger the specific DWARF attributes:

1. **DW_AT_explicit**: Multiple explicit constructors and conversion operators
2. **DW_AT_mutable**: Several mutable data members in different contexts
3. **DW_AT_prototyped**: Numerous function declarations with full prototypes
4. **DW_AT_location**: Variables with different storage durations (global, static, thread_local, local)
5. **DW_AT_lower_bound**: Multi-dimensional arrays and arrays with specified bounds
6. **DW_AT_string_length* attributes**: Various string types including C strings, character arrays, and std::string
7. **DW_AT_is_optional**: std::optional usage
8. **DW_AT_small**: Small bitfield types
9. **DW_AT_ordering**: Enumerations with explicit values
10. **DW_AT_threads_scaled**: thread_local variable

The program ensures all constructs are actually used (not just declared) to prevent optimization from removing debug information. The `main()` function references everything, and we use `volatile` and `__attribute__((used))` strategically to prevent dead code elimination.

Compile with:
