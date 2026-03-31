## Key Design Elements for Triggering Target Code:

1. **Overlapping Loop Bitmaps**: The two inner loops in `generate_loops()` share the exact same `case 1:` block (`arr2[(i + depth) % 100] ^= ...`), creating intersecting but non-subset bitmaps.

2. **Complex Control Flow**: Multiple `switch` statements with fall-through cases create shared basic blocks. The `goto` statements jump to labels outside immediate parent loops, affecting loop membership.

3. **Loop Distribution Candidates**: The mixed computation pattern (compute → conditional access → compute) encourages GCC's loop distribution pass to split loops, altering the loop tree.

4. **Multiple Nesting Depths**: The recursive `generate_loops()` creates loops at different depths, and `main()` calls it with depths 2, 3, and 4.

5. **Volatile Operations**: All array accesses use `volatile` pointers, preventing invariant code motion and keeping loop structures intact.

6. **Early Exits**: Multiple `goto` statements exit loops prematurely to different scopes, creating additional edges in the control flow graph.

## Compilation Recommendations:
