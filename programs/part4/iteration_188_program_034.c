**Key design elements that target the uncovered code:**

1. **High Register Pressure**: 17 local variables of mixed types (int, long, float, double) that are all live across the function call.

2. **Instruction Placement for Movement**: The `critical_value` computation uses values computed before the call and is also used after the call. This creates a situation where the compiler might move this instruction into the save/restore sequence.

3. **Basic Block Structure**: The call to `helper_function` is in the middle of the basic block, with computations both before and after it. This allows for the possibility that when an instruction is moved, it could become the new `BB_END`.

4. **Anti-Optimization Measures**:
   - `volatile` variables and `asm` memory clobbers prevent reordering and elimination
   - Command-line arguments as seeds prevent compile-time evaluation
   - Global variable modification in the helper function creates side effects
   - Loop with varying inputs prevents loop-invariant code motion

5. **Caller-Save Triggering**:
   - Non-inline function calls force actual call instructions
   - Many live values across calls force spilling
   - Mixed data types engage both integer and FP register files

**Compilation and testing:**
