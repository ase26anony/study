## Key Design Elements:

1. **High Register Pressure**: 12+ local variables with complex interdependencies that must stay live across function calls.

2. **Multiple Call Sites**: Three different `noinline` helper functions called with different argument combinations.

3. **Inline Assembly Clobbers**: Explicitly clobber x86-64 caller-saved registers (`rax`, `rcx`, `rdx`, `rsi`, `rdi`, `r8`, `r9`, `r10`, `r11`) and XMM registers to force saves/restores.

4. **Mid-Block Insertion Points**: The inline assembly and function calls are placed between computations, ensuring insertions happen in the middle of basic blocks.

5. **BB_END Update Potential**: The final `helper3` call in the loop could be the block end, and if a save/restore is inserted after it, `BB_END` would need updating.

6. **Anti-Optimization Measures**: 
   - `volatile` variables prevent dead store elimination
   - `noipa` attribute prevents interprocedural analysis
   - Complex computation chains keep variables live
   - Final `printf` ensures side effects

## Compilation Options:

To specifically target the uncovered lines:
