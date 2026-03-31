## Key Design Elements for Triggering the Target Code:

1. **Overlapping Loop Bitmaps**: The `generate_loops` function creates two inner loops that both contain identical `switch` cases 0-2 with the EXACT same handler block. This creates loops whose basic block bitmaps intersect but aren't subsets of each other.

2. **Complex Control Flow**: Multiple `goto` statements, `switch` with fallthrough, and early exits create control flow graphs where basic blocks belong to multiple loops.

3. **Loop Distribution Candidates**: The mixed computation pattern in the second inner loop (computations interspersed with conditionals) encourages GCC's loop distribution pass to split loops.

4. **Recursive Nesting**: The recursive call inside loops creates loops at varying depths in the hierarchy.

5. **Volatile Arrays & Global**: Prevents optimization from simplifying the loop structures too early.

6. **Manual & Pragmatic Unrolling**: Both manual unrolling and `#pragma GCC unroll` create additional basic blocks within loops.

## Recommended Compilation:
