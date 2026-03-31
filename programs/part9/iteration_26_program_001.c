## Key Design Features:

1. **Comprehensive Type Coverage**:
   - **Scalar**: `int`, `float`, `double`, `char`, `bool` (both global and local)
   - **String**: C strings, char arrays, `std::string`
   - **Struct**: Multiple struct types with varying members
   - **Union**: Multiple union types with different member types
   - **Pointer**: Pointers to all type categories, void pointers, double pointers
   - **Array**: Fixed-size arrays of scalars, structs, and unions
   - **Callback**: Function pointers, `std::function`
   - **Lang Struct**: C++ templates, inheritance, STL containers

2. **Complex Type Relationships**:
   - Mutually recursive structs (A ↔ B ↔ C)
   - Nested structs within structs
   - Unions containing structs
   - Templates with multiple instantiations
   - Inheritance hierarchies (single, multiple, virtual)

3. **Optimization Prevention**:
   - `volatile` qualifiers on globals
   - `use()` function with `noinline` attribute
   - All variables are actually used/initialized
   - Complex initialization patterns

4. **C++ Features for TYPE_LANG_STRUCT**:
   - Class templates with different parameters
   - STL containers (`std::vector`, `std::map`, `std::string`)
   - Virtual functions and inheritance
   - Lambda expressions
   - `auto` and `decltype` type deduction

## Compilation Instructions:

To maximize coverage of the `gengtype` code:
