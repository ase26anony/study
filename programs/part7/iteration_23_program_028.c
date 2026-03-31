## Key Design Elements:

1. **High Register Pressure**: 12+ volatile variables used across function calls and inline assembly.

2. **Inline Assembly Clobbers**: Multiple `asm volatile` statements that explicitly clobber x86-64 caller-saved registers (both integer and SSE registers).

3. **Non-Inline Functions**: `foo`, `bar`, `baz`, `qux` marked with `noinline,noipa` to prevent optimization.

4. **Loop Structure**: The 100-iteration loop creates repeated patterns for the register allocator to analyze.

5. **Block-End Candidates**: The final `asm volatile` in the loop body could be the last instruction in a basic block, potentially triggering the `BB_END` update logic.

6. **Result Aggregation**: The `total` variable accumulates results to prevent dead code elimination.

## Compilation Options:
