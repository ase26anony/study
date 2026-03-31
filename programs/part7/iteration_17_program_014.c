**Key features of this test file:**

1. **Comprehensive Type Coverage:**
   - `TYPE_UNDEFINED`: Forward declarations of `struct OpaqueStruct` and `union OpaqueUnion`
   - `TYPE_SCALAR`: `int`, `float`, `double`, `char`, `bool` via typedefs
   - `TYPE_STRING`: `const char*` typedef
   - `TYPE_STRUCT`: Multiple `struct` definitions
   - `TYPE_USER_STRUCT`: `typedef struct { ... }` patterns
   - `TYPE_UNION`: Multiple `union` definitions
   - `TYPE_POINTER`: Various pointer types including function pointers
   - `TYPE_ARRAY`: Fixed-size, incomplete, and multi-dimensional arrays
   - `TYPE_CALLBACK`: Multiple function pointer typedefs
   - `TYPE_LANG_STRUCT`: Structs with GCC attributes like `transaction_safe` and `packed`

2. **Complex Nesting:**
   - Structs containing arrays of pointers
   - Unions containing structs and callbacks
   - Arrays of unions and callbacks
   - The `UltimateStruct` contains one of each major type category

3. **GTY Markers:**
   - Appropriate use of `GTY(())` for garbage-collected types
   - `GTY((skip))` for pointers that shouldn't be traced
   - Applied to structs, unions, and global variables

4. **GCC Compatibility:**
   - Includes necessary GCC headers
   - Uses GCC attributes and conventions
   - Valid C/C++ syntax for GCC codebase

5. **Prevents Dead Code Elimination:**
   - Global variables of each major type
   - `use_all_types()` function that references all types
   - `main()` function for standalone compilation

**To use this for coverage testing:**

1. Place `gtype-test.cc` in the `gcc/` directory of the GCC source tree
2. Build GCC with coverage instrumentation:
