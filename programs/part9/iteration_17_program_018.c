This program creates the necessary conditions to trigger the uncovered bitmap intersection logic:

1. **Complex Loop Nesting Patterns**:
   - `perfect_nesting()` creates perfectly nested loops
   - `partial_overlap()` creates partially overlapping loops with goto jumps
   - `sibling_loops()` creates sibling loops at the same level
   - Mixed entry/exit points via `goto`, `break`, and `continue`

2. **Control Flow Graph Complexity**:
   - Multiple `if-else` statements within loops
   - `switch` statements with multiple cases
   - Loop-carried dependencies with `continue`
   - Multiple condition checks joined by `&&` and `||`

3. **Mixed Loop Types**:
   - `for` loops with complex increments (`i += (i % 3) + 1`)
   - `while` loops with multiple conditions
   - `do-while` loops with early exits
   - Infinite loops (`for(;;)`) with conditional breaks

4. **Function Inlining Boundaries**:
   - Functions marked with `__attribute__((always_inline))`
   - Helper functions called from within loops
   - Recursive function creating loop-like structures

5. **Compiler Optimization Hooks**:
   - `__builtin_expect` for branch prediction
   - `__restrict` qualifiers on pointers
   - `#pragma GCC unroll` directives
   - Array accesses with stride patterns

**Compilation recommendations**:
