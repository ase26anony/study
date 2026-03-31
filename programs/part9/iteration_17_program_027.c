This program creates the necessary complexity to trigger the uncovered bitmap intersection logic:

1. **Complex Loop Nesting Patterns**:
   - `test_perfect_nesting`: Creates perfectly nested loops
   - `test_partial_overlap`: Creates partially overlapping loops with shared blocks
   - `test_sibling_loops`: Creates sibling loops at the same nesting level
   - Uses `goto` statements for non-contiguous block ranges

2. **Control Flow Graph Complexity**:
   - Multiple `if-else` chains within loops
   - `switch` statements with fall-through cases
   - `continue` and `break` at different points
   - `do-while` with nested control flow

3. **Mixed Loop Types**:
   - `for` loops with complex increments (`i += (j % 5) + 1`)
   - `while` loops with compound conditions
   - `do-while` loops with early exits
   - Infinite loops (`for(;;)`) with conditional breaks

4. **Function Inlining Boundaries**:
   - `ALWAYS_INLINE` attribute on helper functions
   - Recursive function creating loop-like CFG
   - Function calls within loops

5. **Compiler Optimization Hooks**:
   - `__restrict` qualifiers on pointers
   - `__builtin_expect` for branch prediction
   - `#pragma GCC unroll` directive
   - Volatile variables to prevent optimization

**Compilation recommendations**:
