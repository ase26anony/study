## Key Design Features:

1. **Partial Basic Block Overlap**: Each test case uses `if/else` statements within outer loops to create situations where inner loops are only executed in certain branches, ensuring their basic blocks are a proper subset of the outer loop's blocks.

2. **Multiple Nesting Levels**: Test Case 1 has three levels (L1, L2, L3), with L2 partially overlapping L1 due to the conditional.

3. **Sibling Loop Relationships**: Test Case 2 creates two inner loops (L5 and L6) at the same nesting level with different containment relationships to the outer loop L4.

4. **Complex Control Flow**: Test Cases 3 and 4 use early continues, switches, and multiple conditionals to create intricate basic block patterns.

5. **Side Effects and Volatility**: The `volatile` qualifier on arrays and the `side_effect()` function ensure operations aren't optimized away. The `rand()` calls and array accesses create necessary side effects.

6. **Function Boundaries**: The `__attribute__((noinline))` on helper functions prevents inlining that could simplify control flow.

## Compilation Recommendations:

For RISC-V hardware loop testing:
