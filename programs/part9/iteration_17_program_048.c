This program creates the necessary complexity to trigger the uncovered bitmap intersection logic:

1. **Complex Loop Nesting Patterns**:
   - Perfect nesting in `test_perfect_nesting()` (inner loop blocks are subsets of outer)
   - Partial overlap in `test_partial_overlap()` (loops share some blocks but have unique ones)
   - Sibling loops in `test_sibling_loops()` (same level, no direct block overlap)
   - Mixed nesting with recursive functions

2. **Control Flow Graph Complexity**:
   - Multiple `if-else` statements creating divergent paths
   - `switch` statements with different `case` labels
   - `goto` statements creating non-contiguous blocks
   - Multiple `continue` and `break` statements at different points

3. **Mixed Loop Types**:
   - `for` loops with complex increments (`i += (i % 3) + 1`)
   - `while` loops with compound conditions
   - `do-while` loops with early exits
   - Infinite loops (`for(;;)`) with conditional breaks
   - Recursive functions creating loop-like structures

4. **Compiler Optimization Hooks**:
   - `__builtin_expect` for branch prediction
   - `__restrict` qualifiers on pointers
   - `#pragma GCC unroll` directives
   - `always_inline` attributes

5. **Execution Flow**:
   - Multiple test functions called from `main()`
   - Actual computations prevent dead code elimination
   - Results accumulated and printed
   - Data dependencies ensure loops aren't optimized away

**Recommended compilation commands**:
