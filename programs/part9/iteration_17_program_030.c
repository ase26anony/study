This program creates the following scenarios to trigger the bitmap intersection logic:

1. **Perfectly Nested Loops**: The `nested_loops_matrix` function creates loops where inner loop blocks are proper subsets of outer loop blocks.

2. **Partially Overlapping Loops**: The `process_array_partial` calls and `sibling_loops` function create loops that share some basic blocks but each has unique blocks.

3. **Sibling Loops**: The two loops in `sibling_loops` exist at the same nesting level but share blocks via `goto` statements.

4. **Non-Contiguous Blocks**: `goto` statements and `break`/`continue` at different points create non-contiguous block ranges.

5. **Mixed Loop Types**: The program uses `for`, `while`, `do-while`, infinite loops, and recursive functions.

6. **Complex Control Flow**: Multiple `if-else` chains, `switch` statements, and conditional `continue`/`break` statements split the control flow graph.

7. **Inlined Functions**: Functions marked with `__attribute__((always_inline))` ensure loops from different functions get merged into the same compilation unit.

**Compilation recommendations:**
