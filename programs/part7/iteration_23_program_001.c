## Key Design Elements:

1. **High Register Pressure**: 12+ live integer variables used across function calls and inline assembly.

2. **Multiple Function Calls**: Three `noinline` helper functions with 6-8 arguments each, forcing argument passing in registers.

3. **Inline Assembly Clobbers**: Two `asm volatile` statements that explicitly clobber:
   - Integer caller-saved registers: `rax`, `rcx`, `rdx`, `rsi`, `rdi`, `r8`, `r9`, `r10`, `r11`
   - Floating-point/SIMD caller-saved registers: `xmm0`-`xmm7`, `xmm8`-`xmm12`
   - Additional registers in second asm to increase pressure

4. **Mid-Block Insertion Points**: The asm statements and function calls are placed between variable uses, ensuring insertions happen in the middle of basic blocks.

5. **Potential BB_END Update**: The last operation in the loop body is `helper3()` call, which could be the block end before insertion.

6. **Loop Structure**: 100 iterations create repeated patterns for the optimization pass to analyze.

7. **Anti-Optimization Measures**:
   - `volatile` variables prevent dead store elimination
   - `noinline` and `noipa` attributes prevent call elimination
   - Result accumulation prevents dead code elimination

## Compilation and Verification:
