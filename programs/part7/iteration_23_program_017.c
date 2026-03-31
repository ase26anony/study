## Key Design Elements:

1. **High Register Pressure**: 12+ live integer variables used across function calls and inline assembly.

2. **Multiple Function Calls**: Three different helper functions with varying numbers of arguments force the compiler to use many argument-passing registers.

3. **Strategic Inline Assembly**: Three `asm volatile` statements with extensive clobber lists for both integer and floating-point caller-saved registers:
   - First asm: Clobbers rax, rcx, rdx, rsi, rdi, r8, r9, r10, r11, and xmm0-xmm5
   - Second asm: Additional clobbers including rbx and xmm6-xmm9
   - Third asm: Positioned to potentially be a BB_END candidate

4. **Loop Structure**: The 100-iteration loop creates repeated patterns for the compiler to analyze, increasing chances of triggering the specific insertion logic.

5. **Preventing Optimizations**:
   - `volatile` variables prevent dead store elimination
   - `__attribute__((noinline, noipa))` prevents inlining and inter-procedural analysis
   - Result aggregation prevents dead code elimination

6. **BB_END Trigger**: The final inline assembly statement is placed at a position where it could be the last instruction in a basic block, potentially making `BB_END(bb) == insn` true.

## Compilation and Analysis:

To compile and analyze the coverage:
