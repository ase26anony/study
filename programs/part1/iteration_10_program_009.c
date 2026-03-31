## Key Design Elements for Triggering the Target Code:

1. **Partially Overlapping Loop Blocks**: The two inner loops in `complex_loop_hierarchy` share the switch cases 0-2 but have different unique cases, creating bitmaps that intersect but aren't subsets.

2. **Complex Control Flow**: Multiple `switch` statements, `goto` jumps to labels outside immediate loops, and conditional breaks create basic blocks that belong to multiple loops.

3. **Loop Transformations**: Manual unrolling with `#pragma GCC unroll`, loop distribution pattern (computation → condition → computation), and recursive nesting.

4. **Non-Constant Bounds**: All loop bounds use `volatile` variables or `rand()` calls, preventing constant propagation.

5. **Multiple Loop Candidates**: The recursive `generate_loops_recursive` creates loops at varying depths (0-3), populating the compiler's loop tree with many candidates for parent-child analysis.

6. **Anti-Optimization**: `volatile` arrays, pointer arithmetic with aliasing, and `__attribute__((noinline, noipa))` prevent premature optimization.

## Recommended Compilation:
