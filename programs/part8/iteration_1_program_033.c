This program generates comprehensive test coverage by:

## Key Features:

1. **Balanced Construct Nesting** (`Requirement 1`):
   - Multi-dimensional arrays: `int matrix[3][4][5];`
   - Function pointers: `void (*handlers[10])(int, char*);`
   - Complex nested structures with all delimiter types
   - Pointer-to-function syntax: `int (*(*complex_callback)(void*(*)(int)))(char*);`

2. **Unbalanced Edge Cases** (`Requirement 2`):
   - Missing closing brackets: `int values[10;`
   - Missing closing braces in struct definitions
   - Recovery testing with valid syntax after errors

3. **Comments and Macros Interleaving** (`Requirement 3`):
   - Block comments with nested `/* */`
   - Line comments with delimiter-like characters
   - Macro definitions: `#define CREATE_PAIR(T) struct { T first; T second; }`
   - Multi-line macros with balanced delimiters

4. **GT File Specific Constructs** (`Requirement 4`):
   - `%typedef`, `%struct`, `%union`, `%enum` annotations
   - `GC_ROOT()` macro with balanced delimiters
   - Embedded delimiters within GT annotations

5. **Multiple Top-Level Declarations** (`Requirement 5`):
   - Multiple structs, unions, enums, and typedefs
   - Varied type declarations to force repeated parser passes

6. **Two Test Files**:
   - `test_complex.gt`: Comprehensive balanced constructs
   - `test_edge_cases.gt`: Edge cases and error recovery

## Compilation and Execution:
