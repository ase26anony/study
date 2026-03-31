**Key features that trigger each tree node type:**

1. **IDENTIFIER_NODE**: Multiple global variables (`global_extern_var`, `global_static_var`, `global_regular_var`), function names, parameter names, and local variables throughout.

2. **TREE_VEC**: 
   - Multi-dimensional array `multi_dimensional_array[3][4][5]`
   - Array initializers with designators: `int sparse_array[10] = { [2] = 42, [5] = 99, [9] = 77 }`
   - In C++ mode: Template instantiations implicitly through standard library includes

3. **TREE_BINFO** (C++ only):
   - Class hierarchy with `BaseClass`, `DerivedClass`, `SecondDerived`
   - Virtual methods and dynamic casts
   - Virtual function calls through base class pointers

4. **SSA_NAME**:
   - Complex conditional assignments in loops: `temp = (i % 2 == 0) ? array[i] * 2 : array[i] + local_in_block`
   - Multiple assignments to `result` in different control flow paths
   - Phi nodes created by if-else structures inside loops

5. **BLOCK**:
   - Explicit blocks with `{ }` throughout
   - Nested blocks with local variables
   - `goto` statements jumping between blocks
   - Labels and block scopes

6. **CONSTRUCTOR**:
   - Struct initialization: `struct ComplexStruct my_struct = { .a = argc, ... }`
   - Array initializers
   - Return values from `recursive_struct_builder` function

7. **OMP_CLAUSE**:
   - Multiple OpenMP pragmas with various clauses: `private`, `firstprivate`, `shared`, `reduction`, `collapse`, `schedule`, `lastprivate`, `atomic`, `sections`
   - Nested OpenMP directives

**Compilation recommendations for maximum coverage:**
