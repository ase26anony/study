**Key features that target the uncovered lines:**

1. **High Register Pressure**: Multiple functions with many live variables of different types (int, double, pointers) in loops.

2. **Pure Functions**: `pure_compute()` marked with `__attribute__((const))` to encourage rematerialization instead of spilling.

3. **Complex Control Flow**: Uses `goto`, nested loops, and conditionals to create non-trivial CFG that inhibits CSE and LICM.

4. **Mixed Data Types and Addressing**: Uses structs with different-sized members, pointer arithmetic, and array indexing to create varied register modes.

5. **Inline Assembly**: Creates artificial register pressure with `asm volatile` constraints.

6. **Optimization Pragmas**: Uses `optimize("O2")` and `optimize("O3")` attributes to ensure the early remat pass runs.

7. **Noinline Attributes**: Prevents function inlining which would reduce register pressure.

**Compilation recommendations:**
