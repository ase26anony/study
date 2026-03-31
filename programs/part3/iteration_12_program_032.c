## Key Features Targeting the Uncovered Lines:

1. **Nested Loop Structure**: 
   - Outer loop (Loop Set A) contains two inner loops with shared condition block (`shared_cond`)
   - Creates bitmap intersections where inner loops share blocks with outer loop but also have unique blocks

2. **Complex Control Flow**:
   - Multiple `if/else` branches within loops
   - Mix of `break` and `continue` statements
   - Different loop types (`for` and `while`)
   - Early exit conditions

3. **Shared and Unique Blocks**:
   - `double_val()` function called from multiple loops
   - `conditional_mod()` function creates unique blocks
   - Shared condition checks (`shared_cond`) across loops
   - Different loop bodies create complement conditions

4. **Variable Loop Bounds**:
   - Bounds depend on command-line arguments
   - Mix of small (5) and large (1024) iteration counts
   - Loop-invariant computations that get hoisted

5. **Compiler Hints**:
   - `register` and `volatile` qualifiers
   - Array stores prevent dead code elimination
   - Final checksum computation ensures all code is live

## Compilation Commands:
