## Key Design Elements:

1. **Overlapping Loop Bitmaps**: 
   - The `generate_loops` function creates two inner loops that share the same `switch` statement and `if ((idx % 3) == 0)` block
   - This creates loops whose basic block bitmaps intersect but neither is a subset of the other

2. **Complex Control Flow**:
   - Multiple `switch` statements with shared `default:` cases
   - `goto` statements that jump outside immediate parent loops (`early_exit`, `adjust_global`)
   - Mixed loop types (`for`, `while`) with different structures

3. **Loop Transformations**:
   - Manual unrolling in the first inner loop
   - `#pragma GCC unroll 2` in the second inner loop
   - The `fission_loops` function has a loop designed to be split by distribution passes

4. **Non-Constant Bounds**:
   - All loop bounds use `volatile` variables or `rand()`-like computations
   - Early exits based on array values

5. **Multiple Loop Candidates**:
   - Recursive `generate_loops` called with depths 2, 3, 4
   - Different loop patterns (`triple_nest`, `fission_loops`)
   - Multiple iterations in `main()` to populate the loop tree

6. **Anti-Optimization**:
   - `__attribute__((noinline, noipa, optimize("O3")))` on all functions
   - `volatile` arrays and pointers
   - Pointer arithmetic with aliasing concerns

## Compilation Recommendations:
