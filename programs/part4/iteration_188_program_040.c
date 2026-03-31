**Key design elements that target the uncovered code:**

1. **High Register Pressure**: 12 variables of mixed types (int, long, float, double) that are all live across the function call.

2. **Non-inlined Call**: `helper_function` is marked `noinline` to ensure a real call instruction is generated.

3. **Instruction Placement Opportunities**: 
   - Computations using `v1`, `v2`, `v3` both before and after the call
   - The `extra`, `f_extra`, and `d_extra` computations after the call use values computed before the call, creating opportunities for the compiler to move instructions into the save/restore sequence

4. **Basic Block Structure**: The call to `helper_function` is in the middle of a basic block with instructions both before and after it, allowing `BB_END` to potentially need updating if an instruction is moved.

5. **Memory Barriers**: `asm volatile` statements prevent excessive reordering, encouraging explicit save/restore sequences.

6. **Loop Structure**: The loop in `main` causes repeated execution of the caller-save logic, increasing coverage probability.

**Compilation and execution:**
