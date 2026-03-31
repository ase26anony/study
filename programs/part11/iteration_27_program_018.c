**Key Design Elements:**

1. **Explicit Register Variables**: Uses `register ... asm ("r10")` and `register ... asm ("r11")` to bind variables to call-clobbered registers on x86_64.

2. **Basic Block Boundaries Created By**:
   - `if-else` statements with calls as the last operation before `return` or falling through
   - Loop with call at the end of the loop body
   - `switch` statement with calls at the end of case blocks

3. **Register Pressure**:
   - Many local variables (`var1` through `var12`) competing for registers
   - Complex expressions preventing easy optimization
   - Multiple register variables that must be preserved across calls

4. **Anti-Optimization**:
   - `volatile` globals prevent dead code elimination
   - `noinline` and `noclone` attributes ensure real function calls
   - Results accumulated in `total` and printed

**Compilation and Verification:**
