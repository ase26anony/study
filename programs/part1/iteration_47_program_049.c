This test program includes:

1. **IDENTIFIER_NODE**: Many distinct variable names, `__label__` declarations, and type names used with `__builtin_types_compatible_p`.

2. **TREE_VEC**: Functions with many parameters, VLAs, complex attribute lists, and potentially in the representation of array bounds or template arguments.

3. **TREE_BINFO**: A complex C++ class hierarchy with multiple inheritance, virtual bases, virtual functions, `dynamic_cast`, and pointer-to-member usage.

4. **SSA_NAME**: The `ssa_heavy_function` contains complex control flow with loops, conditionals, and self-modifying assignments that force SSA form generation.

5. **BLOCK**: `nested_blocks_function` creates deeply nested lexical scopes with variables declared in each block.

6. **CONSTRUCTOR**: `use_constructors` demonstrates various aggregate initializers including arrays, structs with designated initializers, and compound literals.

7. **OMP_CLAUSE**: `openmp_kernel` uses multiple OpenMP pragmas with various clauses including `reduction`, `private`, `shared`, `map`, `depend`, and others.

Compile with:
