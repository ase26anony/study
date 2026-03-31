## Key Design Elements:

1. **High Register Pressure**: Multiple functions with many live variables of different types (int, double, pointers) within loops.

2. **Rematerialization Candidates**:
   - `pure_helper()` calls marked with `__attribute__((const))`
   - Complex addressing computations in `addr_helper()`
   - Type conversions between int and double

3. **Control Flow Complexity**:
   - Nested loops in `hot_function_2`
   - `goto` statements in `hot_function_3`
   - Switch statements and conditional branches

4. **Mixed Data Types and Addressing**:
   - Integer and floating-point operations
   - Array indexing with complex expressions
   - Pointer arithmetic

5. **Compiler Hints**:
   - `register` keyword for critical variables
   - `#pragma GCC unroll` to increase pressure
   - Inline assembly to force register usage
   - `__attribute__((noinline))` to keep functions separate

## Compilation Recommendations:
