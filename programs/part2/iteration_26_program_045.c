## Key Features That Trigger the Uncovered Code:

1. **Selective Scheduler Activation**: The code uses `-fsel-sched-pipelining` and `-fsel-sched-pipelining-outer-loops` flags which enable the selective scheduler whose debug output contains the uncovered lines.

2. **Complex Loop Structures**: 
   - Nested loops with inner loop count dependent on outer index
   - Loop-carried dependencies through `data[i-1]`
   - `#pragma GCC unroll` creating large basic blocks

3. **Mixed Operations**:
   - SIMD intrinsics (SSE4.2 operations)
   - Inline assembly with register clobbers
   - Function calls within loops
   - Conditional moves via ternary operators

4. **Complex Control Flow**:
   - Computed goto creating indirect branches
   - Switch statements with sparse cases
   - Multiple basic block boundaries

5. **Memory Access Patterns**:
   - Non-contiguous access through struct fields
   - Pointer arithmetic and casting
   - Mixed data types (int, double, char, float)

6. **Compiler Pragmas and Attributes**:
   - `__attribute__((optimize(...)))` to force specific optimizations
   - Multiple optimization levels within the same program

## Compilation and Execution:
