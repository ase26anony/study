## Key Design Elements:

1. **Overlapping Loop Bitmaps**: The two inner loops in `generate_loops` (when `depth <= 0`) share the same `if (local_seed % 3 == 0)` and switch statement blocks, but have different unrolling patterns, creating partially overlapping bitmaps.

2. **Complex Control Flow**: Multiple `switch` statements with shared handler blocks (`if (arr3[j] < 0) { arr3[j] = 0; }`) create basic blocks that belong to multiple loops but aren't perfectly nested.

3. **Loop Transformations**: Manual unrolling (4 operations in sequence), `#pragma GCC unroll`, and loop distribution candidates (computation → conditional access → computation pattern).

4. **Non-Constant Bounds & Early Exits**: Volatile bounds, `rand()`-based limits, and `goto` statements that exit to labels outside immediate parent loops.

5. **Prevention of Optimization**: `__attribute__((noinline, noipa))`, volatile pointers, complex pointer arithmetic with modulo operations to create aliasing concerns.

6. **Multiple Loop Depths**: Recursive `generate_loops` function creates loops at depths 2, 3, and 4, called multiple times from `main`.

## Compilation Recommendations:
