## Key Design Elements:

1. **Nested Loops with Complex Control Flow**:
   - Multiple levels of `for`, `while`, and `do-while` loops
   - Early exits via `break`, `return`, and `goto`
   - Multiple entry points using `goto` labels

2. **Partially Overlapping Loop Bodies**:
   - `test_powerpc_nested_loops()`: Loop B fully inside Loop A (subset relationship)
   - `test_arm_partial_overlap()`: Loops C and D intersect but each has unique blocks
   - `test_disjoint_loops()`: Loops E and F are completely disjoint

3. **Hardware Loop Analysis Triggers**:
   - Counted loops with constant bounds
   - Linear array access patterns
   - Integer and floating-point accumulations
   - Memory barriers with `asm volatile`

4. **Architecture-Specific Targeting**:
   - PowerPC-specific function with `__attribute__((target("arch=powerpc")))`
   - ARM-specific function with `__attribute__((target("arch=arm")))`
   - Generic functions for baseline testing

5. **Complex Control Flow Within Loops**:
   - `switch` statements with multiple cases
   - Computed `goto` via label arrays
   - Function calls to `noinline` helpers
   - Nested `if-else` chains

## Compilation Recommendations:
