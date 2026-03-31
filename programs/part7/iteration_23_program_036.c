**Key design elements that target the uncovered lines:**

1. **High Register Pressure**: 12+ local variables with complex interdependencies force register spilling.

2. **Inline Assembly Clobbers**: Explicitly clobber x86-64 caller-saved registers (`rax`, `rcx`, `rdx`, `rsi`, `rdi`, `r8`, `r9`, `r10`, `r11`) to force save/restore insertion.

3. **Non-Inline Function Calls**: `NOINLINE` attribute ensures calls remain and their register-clobbering effects are preserved.

4. **Mid-Block Insertion Points**: The inline assembly statements are placed between variable uses, not at block boundaries, increasing the chance that inserted save/restore instructions have both previous and next instructions in the chain.

5. **Potential BB_END Updates**: The function calls and inline assembly at the end of loop iterations and before returns create scenarios where the instruction before insertion (`insn`) could be the current block end.

6. **Loop Structure**: The loop creates repeated patterns for the optimization pass to analyze, with varying values preventing constant propagation.

**Compilation and verification:**
