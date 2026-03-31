This program creates the necessary complexity to trigger the uncovered bitmap intersection logic:

1. **Complex Loop Nesting Patterns**:
   - `perfect_nesting_test`: Creates perfectly nested loops
   - `overlapping_loops_test`: Creates partially overlapping loops with shared blocks
   - Multiple sibling loops at the same nesting level
   - Non-contiguous blocks via `goto` and multiple `break` statements

2. **Control Flow Graph Complexity**:
   - `switch` statements inside loops with fall-through cases
   - Multiple `if-else` chains creating divergent paths
   - `continue` statements at different points in loop bodies
   - Early exits with `break` at different nesting levels

3. **Mixed Loop Types**:
   - `for` loops with variable increments (`i += (i % 3) + 1`)
   - `while` loops with complex conditions (`i < n && j < n`)
   - `do-while` loops with early exits
   - Infinite loops (`for(;;)`) with conditional breaks

4. **Function Inlining Boundaries**:
   - Functions marked with `__attribute__((always_inline))`
   - Helper functions containing loops called from other loops
   - Recursive functions creating loop-like structures

5. **Compiler Optimization Hooks**:
   - `__builtin_expect` for branch prediction
   - `__restrict` qualifiers on pointer arguments
   - `#pragma GCC unroll` directives
   - Array accesses with stride patterns

**Compilation recommendations**:
