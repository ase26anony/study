## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The `generate_loops` function creates two inner loops that share:
   - The same conditional block `if ((j ^ i) & 0x3)`
   - The same switch cases (cases 3 and 5)
   This creates loops whose basic block bitmaps intersect but neither is a subset of the other.

2. **Complex Control Flow**: 
   - Multiple `switch` statements with overlapping cases
   - `goto` statements that jump to shared labels (`early_exit`, `shared_handler`)
   - Early exits with `break` and `goto` that exit to outer scopes

3. **Loop Transformations**:
   - Manual unrolling (4 iterations in first inner loop)
   - `#pragma GCC unroll 2` directive
   - Loop distribution candidates (computation → conditional memory access → more computation)

4. **Non-Constant Bounds**:
   - Loop bounds using `rand() % N + M`
   - `volatile` variables for bounds to prevent constant propagation
   - Data-dependent loop termination conditions

5. **Multiple Loop Hierarchies**:
   - Recursive `generate_loops` creates varying depths (2, 3, 4)
   - `overlapping_loop_pattern` creates non-nested loops that share basic blocks
   - Multiple calls in `main` with different parameters

6. **Optimization Prevention**:
   - `__attribute__((noinline, noipa, optimize("O3")))` on key functions
   - `volatile` arrays and pointers
   - Complex pointer arithmetic and aliasing
   - Global `volatile control` variable modified between calls

## Compilation Recommendations:
