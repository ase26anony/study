This program creates the necessary complexity to trigger the uncovered bitmap intersection logic:

1. **Complex Loop Nesting Patterns**:
   - Perfectly nested loops in `process_inner_loop` called from outer loops
   - Partially overlapping loops in `test_partial_overlap` with shared blocks
   - Sibling loops in `test_sibling_loops` that share no blocks
   - Multiple entry points via `goto` labels in `test_multiple_entries`

2. **Control Flow Graph Complexity**:
   - `if-else` statements with `continue` at different points
   - `switch` statements with fall-through and goto between cases
   - Multiple loop exit points via `break` and `goto`

3. **Mixed Loop Types**:
   - `for` loops with complex increments (`i += (i % 5) + 1`)
   - `while` loops with multiple conditions
   - `do-while` loops with embedded switches
   - Infinite loops (`for(;;)`) with conditional breaks

4. **Function Inlining Boundaries**:
   - `__attribute__((always_inline))` on helper functions
   - Recursive function creating loop-like structures
   - Multiple function calls within loops

5. **Compiler Optimization Hooks**:
   - `__builtin_expect` for branch prediction
   - `__restrict` qualifiers on pointers
   - `#pragma GCC unroll` directives
   - Array accesses with predictable patterns

**Compilation recommendations**:
