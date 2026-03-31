This program systematically exercises all the required features:

1. **Explicit Constructors**: Three classes with `explicit` constructors and conversion operators
2. **Optional Attributes**: Uses `[[maybe_unused]]` and `std::optional` extensively
3. **Mutable Members**: `MutableStruct` with `mutable` fields and bit-fields
4. **Fortran Arrays**: `FortranArray` with lower bounds, ordering, and segment simulation
5. **String Types**: `ExplicitLengthString` with length tracking and `PictureString`
6. **Function Prototypes**: Mix of old-style declarations and full prototypes
7. **Thread-Local Storage**: `thread_local` variables with scaled access patterns

**Compilation and Execution:**
