**Key Design Elements:**

1. **High Register Pressure**: 12+ integer variables with complex interdependencies ensure most architectural registers are occupied.

2. **Multiple Function Calls**: Three different `noinline` functions with varying numbers of arguments force the compiler to use the standard calling convention.

3. **Inline Assembly Clobbers**: Explicitly clobber both integer and floating-point caller-saved registers at strategic points between variable uses.

4. **Basic Block Structure**: The code is structured so that:
   - Function calls and asm statements occur in the middle of computations
   - The last operation in the loop body (`func3` call) could be the `BB_END` candidate
   - Insertions would need to happen between existing instructions

5. **Anti-Optimization Measures**:
   - `volatile` variables prevent dead store elimination
   - `noipa` attribute prevents interprocedural analysis
   - Complex computations keep variables live across calls
   - Final `printf` ensures no dead code elimination

**Compilation and Testing:**
