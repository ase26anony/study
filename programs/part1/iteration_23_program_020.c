**Key features that trigger each tree node type:**

1. **IDENTIFIER_NODE**: 
   - Label address (`&&my_label`)
   - `__FUNCTION__` in static assert
   - `offsetof` with field identifier

2. **TREE_VEC**:
   - Vector extension type (`v4si`)
   - Lambda with many parameters (parameter list may use TREE_VEC)

3. **TREE_BINFO**:
   - C++ class hierarchy with multiple inheritance
   - `dynamic_cast` between base classes
   - `typeid` operator
   - Virtual function calls

4. **SSA_NAME**:
   - Loop with conditional assignment creating phi nodes
   - Variable `x` with different assignments in if/else branches

5. **BLOCK**:
   - Nested compound statements with local variables
   - Label inside innermost block

6. **CONSTRUCTOR**:
   - Designated struct initializer
   - Compound literal array
   - Nested struct initialization

7. **OMP_CLAUSE**:
   - OpenMP pragmas with `private`, `reduction`, `schedule` clauses
   - OpenMP sections with `firstprivate`

8. **Tree inspection triggers**:
   - `__builtin_constant_p()` on aggregate initializer
   - `__builtin_types_compatible_p()` on vector and struct types
   - `static_assert` forcing compile-time evaluation

**Compilation recommendations:**
