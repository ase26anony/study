## Key Design Elements:

1. **Overlapping Loop Blocks**: The `if ((i + j) % 3 == 0)` condition appears in multiple loops at different nesting levels, creating basic blocks that belong to multiple loops but aren't perfectly nested.

2. **Switch Statements with Shared Targets**: The `switch ((i + m) % 5)` creates control flow that can jump to shared computation blocks from different loops.

3. **Manual and Compiler Unrolling**: Both `#pragma GCC unroll 4` and manual unrolling (4 iterations explicitly written) create multiple basic blocks that may be shared.

4. **Early Exits with `goto`**: The `goto early_exit` and `goto outer_loop_continue` create edges from inner loops to outer blocks, affecting loop membership.

5. **Recursive Loop Generation**: The `recursive_loop_generator` creates loops at varying depths, populating the compiler's loop tree with many candidates for comparison.

6. **Volatile Arrays and Pointers**: All arrays are declared `volatile` and passed to functions, preventing optimization of memory operations.

7. **Data-Dependent Loop Bounds**: Loops use `(control % N) + M` for bounds, making them opaque to the compiler's static analysis.

## Compilation Recommendations:
