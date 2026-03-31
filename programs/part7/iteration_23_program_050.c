## Key Design Elements:

1. **High Register Pressure**: 12+ volatile variables with complex interdependencies force register spilling.

2. **Multiple Function Calls**: Three different helper functions with varying numbers of arguments create multiple call sites requiring caller-save operations.

3. **Inline Assembly Clobbers**: Explicit clobbering of x86-64 caller-saved registers (`rax`, `rcx`, `rdx`, `rsi`, `rdi`, `r8`-`r11`) and XMM registers forces the compiler to save/restore around these points.

4. **Mid-Block Insertion Points**: The inline assembly statements are placed between computations, ensuring they're not at basic block boundaries.

5. **Potential BB_END Update**: The final inline assembly in the loop could be the last instruction in a basic block, potentially triggering the `BB_END` update logic.

6. **Anti-Optimization Measures**: 
   - `volatile` variables prevent dead store elimination
   - `noinline` and `noipa` attributes prevent function call optimization
   - Result accumulation prevents dead code elimination

## Compilation Options:
