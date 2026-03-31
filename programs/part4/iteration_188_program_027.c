**Key design elements that target the uncovered code:**

1. **High Register Pressure**: 17 variables of mixed types (int, long, float, double) are declared and used both before and after the `helper_function` call, forcing the compiler to spill caller-saved registers.

2. **Live Values Across Call**: All variables are used in computations after the call, creating true liveness that requires preservation.

3. **Instruction Placement Opportunities**: The `extra_calc` variable computation `(v1 * v2) + (v3 - v4)` uses values computed before the call but is only used after. This gives the compiler flexibility to schedule it around the save/restore sequence.

4. **Basic Block Structure**: The `if (result % 7 == 0)` check after the call in `main()` ensures the call isn't at the end of its basic block, allowing for the `BB_END` update logic to be triggered.

5. **Anti-Optimization Measures**:
   - `volatile` variables and `asm` memory clobbers prevent reordering and elimination
   - Non-inline attributes ensure actual function calls
   - Global side effects in `helper_function`
   - Loop with varying inputs prevents constant propagation

**Compilation and testing:**
