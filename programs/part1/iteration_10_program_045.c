## Key Design Elements:

1. **Overlapping Loop Blocks**: The `shared_block` label and `goto` statements create basic blocks that belong to multiple loops but aren't perfectly nested subsets.

2. **Complex Switch Statements**: The `switch` with fall-through creates multiple basic blocks that are conditionally entered from different loops, creating partial bitmap overlaps.

3. **Manual and Pragmatic Unrolling**: Both manual unrolling and `#pragma GCC unroll` create additional basic blocks within loops.

4. **Early Exits with `goto`**: The `goto finish;` exits a loop to a block outside its immediate parent, affecting bitmap complement calculations.

5. **Recursive Loop Generation**: The `generate_loops()` function creates loops at different nesting depths, populating the compiler's loop tree.

6. **Volatile Arrays and Pointers**: These prevent optimization and keep memory operations opaque to early passes.

7. **Data-Dependent Loop Bounds**: Using `rand()` and `volatile` variables for loop bounds prevents constant propagation.

## Compilation Recommendations:
