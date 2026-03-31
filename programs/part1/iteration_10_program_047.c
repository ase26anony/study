## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The two inner loops in `generate_loops()` share the `switch` statement with `case 1:` handler, creating intersecting but not fully contained block sets.

2. **Complex Control Flow**: 
   - `switch` statements with fall-through to shared blocks
   - `goto` statements for early exits
   - Multiple conditional blocks inside loops

3. **Loop Transformations**:
   - Manual unrolling in inner loops
   - `#pragma GCC unroll` directive
   - Loop distribution candidate in `nested_switch_loops()`

4. **Non-Constant Bounds**: All loops use `volatile` or `rand()`-based bounds

5. **Multiple Nesting Depths**: Recursive `generate_loops()` creates loops at depths 2, 3, and 4

6. **Anti-Optimization**: 
   - `__attribute__((noinline, noipa, optimize("O3")))`
   - `volatile` arrays and variables
   - Pointer arithmetic with aliasing

## Compilation Recommendations:
