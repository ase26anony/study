**Key features of this test file:**

1. **Comprehensive Type Coverage:**
   - **TYPE_UNDEFINED:** `struct OpaqueStruct;`, `union OpaqueUnion;`
   - **TYPE_SCALAR:** `int`, `float`, `double`, `char`, `bool`, enums, bitfields
   - **TYPE_STRING:** `const char*` with string literal
   - **TYPE_STRUCT:** Multiple `struct` definitions
   - **TYPE_USER_STRUCT:** `typedef struct { ... } UserStruct;`
   - **TYPE_UNION:** Multiple `union` definitions
   - **TYPE_POINTER:** Various pointers to all base types
   - **TYPE_ARRAY:** Fixed-size, incomplete, and multi-dimensional arrays
   - **TYPE_CALLBACK:** Function pointers with different signatures
   - **TYPE_LANG_STRUCT:** Structs with GCC attributes (`transaction_safe`, `packed`)

2. **Complex Nesting:**
   - Structs containing arrays of pointers to unions
   - Unions containing structs and callbacks
   - Self-referential pointers
   - Arrays of function pointers
   - Nested structs within unions

3. **GTY Markers:**
   - All major structs/unions marked with `GTY(())`
   - Some pointers marked with `GTY((skip))` to show different annotations
   - Function marked with `GTY((user))`

4. **GCC Compatibility:**
   - Includes necessary GCC headers
   - Follows GCC coding patterns
   - Uses GCC extensions (attributes)
   - Valid C++ that can be compiled as part of GCC

**To use this for coverage testing:**

1. Place `gtype-test.cc` in the `gcc/` directory of the GCC source tree
2. Add it to the build by modifying `gcc/Makefile.in` or building manually
3. Configure GCC with coverage flags:
