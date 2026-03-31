## Key Design Elements for Coverage:

1. **Multiple Nested Loop Structures**:
   - Outer loop containing two inner loops (for and while) with shared condition blocks
   - Sequential loops with different structures (for, while, do-while)
   - Small inner loops within larger loops

2. **Complex Control Flow**:
   - Multiple `if` statements with function calls in conditions
   - `break` and `continue` statements at different points
   - `switch` statement inside a loop
   - Shared condition blocks between loops

3. **Bitmap Intersection Triggers**:
   - `double_value()` and `maybe_halve()` functions called from multiple loops
   - Shared condition checks (`shared_condition`) across loops
   - Loop-invariant computations that create pre-header blocks

4. **Variable Loop Bounds**:
   - Bounds depend on `argc` and seed values
   - Mix of small fixed bounds (3, 4) and larger bounds (1024)
   - Dynamic bounds prevent complete unrolling

## Compilation and Testing:
