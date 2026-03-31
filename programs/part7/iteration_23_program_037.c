## Key Design Elements:

1. **High Register Pressure**: 12+ volatile variables with complex interdependencies create many live values across calls.

2. **Multiple Call Sites**: Three different non-inline functions called with different argument combinations.

3. **Explicit Register Clobbering**: Inline assembly with explicit clobber lists for x86-64 caller-saved registers (both integer and XMM registers).

4. **Mid-Block Insertion Points**: The asm statements are placed between computations, ensuring save/restore insertions occur in the middle of basic blocks.

5. **BB_END Update Potential**: The final asm statement in each loop iteration could be the block end when followed by a save/restore insertion.

6. **Loop Structures**: Both outer and inner loops create repeated patterns for the optimization pass to analyze.

## Compilation Options:
