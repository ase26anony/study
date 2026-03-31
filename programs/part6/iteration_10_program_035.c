This program systematically exercises all the required features:

1. **Explicit Constructors**: `ExplicitInt`, `ExplicitString`, and `MultiExplicit` classes with `explicit` constructors and conversion operators.

2. **Optional Attributes**: Uses `[[maybe_unused]]` on parameters, variables, and structured bindings, combined with `std::optional` types.

3. **Mutable Members**: `MutableStruct` contains `mutable` data members, nested within `ComplexUnion`, with bit-fields for string length attributes.

4. **Fortran-like Arrays**: `FortranArray` template stores lower bounds, ordering, and segment information, simulating Fortran array descriptors.

5. **String Types**: `ExplicitLengthString` stores explicit length, `PictureString` stores picture clauses, with related bit-field attributes.

6. **Function Prototypes**: All functions have full prototypes, with GCC's `__attribute__((prototype))` if available.

7. **Thread-Local Storage**: Uses `thread_local` variables with scaling based on thread ID, accessed from multiple threads.

The `main()` function creates instances of all types, performs operations that require explicit conversions, uses optional values, modifies mutable members, accesses arrays with non-zero lower bounds, uses custom string types, calls prototyped functions, and launches threads that perform scaled thread-local operations.

Compile with the recommended flags to maximize DWARF generation:
