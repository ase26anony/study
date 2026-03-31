This program creates the necessary conditions to trigger the uncovered bitmap intersection logic:

1. **Complex Loop Nesting Patterns**:
   - Perfectly nested loops (inner loops within `complex_nested_loops`)
   - Partially overlapping loops (`process_inner_loop` called from different contexts)
   - Sibling loops (two consecutive loops in `test_all_patterns`)
   - Non-contiguous blocks created via `goto` statements and early exits

2. **Control Flow Graph Complexity**:
   - Multiple `if-else` statements within loops
   - `switch` statements inside loops with different `case` blocks
   - `continue` and `break` statements at different nesting levels
   - Loop-carried dependencies with variable increments

3. **Mixed Loop Types**:
   - `for` loops with complex increments (`i += (i % 3) + 1`)
   - `while` loops with compound conditions (`j < 5 && (arr[idx + i] < 100 || i > 2)`)
   - `do-while` loops with conditional breaks
   - Infinite loops (`for(;;)`) with multiple exit points

4. **Function Inlining Boundaries**:
   - `__attribute__((always_inline))` forces inlining of key functions
   - Helper functions (`process_inner_loop`) called from within loops
   - Recursive function (`recursive_loop`) creating loop-like structures

5. **Compiler Optimization Hooks**:
   - `__builtin_expect` for branch prediction
   - `__restrict` qualifiers on pointer arguments
   - `#pragma GCC unroll` directives
   - Array accesses with stride patterns

**Compilation recommendations**:
