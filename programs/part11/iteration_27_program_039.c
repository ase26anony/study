**Key design elements that trigger the uncovered code:**

1. **Explicit register variables**: Using `register ... asm ("r12")` (and r13, r14) binds variables to specific call-clobbered registers that must be saved/restored across calls.

2. **Basic block boundaries at calls**: 
   - Calls immediately before `return` statements
   - Calls as the last statement in `if` blocks before `break` or implicit fall-through
   - Calls at the end of loop bodies before the loop latch

3. **Register pressure**: Multiple local variables (`v1`-`v8`) compete for registers, forcing spills.

4. **Live ranges across calls**: `reg_var` is used before and after function calls, requiring preservation.

5. **Multiple calling contexts**: Different helper functions with different parameter counts create varied calling sequences.

**Compilation and verification:**
