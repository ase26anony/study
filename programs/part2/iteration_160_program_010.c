This program creates the specific loop patterns needed to trigger the uncovered lines:

1. **Proper Subset Relationship** (lines 432-433): Created by Loop B fully inside Loop A in `test_powerpc_nested_loops()`.

2. **Intersecting but Neither Subset** (lines 434-435): Created by Loop E and Loop F in `test_arm_complex_loops()` where they share some blocks but each has unique blocks.

3. **Disjoint Loops** (line 429-430): Created by the disjoint loops in `test_mixed_loop_patterns()`.

4. **Complex Control Flow**: Uses `goto`, `switch`, early `break`/`return`, function calls, and memory barriers to create complex basic block patterns.

5. **Hardware Loop Candidates**: Includes simple counted loops with linear array access that should trigger hardware loop analysis.

To compile and test with coverage:
