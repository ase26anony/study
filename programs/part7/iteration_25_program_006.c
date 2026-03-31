## Important notes:
1. **Index is 0-based**: `__type_pack_element<1, ...>` selects the **second** type
2. **Compile-time selection**: This happens entirely at compile time
3. **Compiler-specific**: `__type_pack_element` is not standard C++ - it's a compiler extension
4. **Error if out of bounds**: If you pass fewer than 2 types, this will cause a compilation error

## Standard C++ alternatives:
In standard C++20, you could use:
