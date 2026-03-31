**Key Design Elements:**

1. **Multiple Call Sites at Block Boundaries:**
   - Call at end of `if` block (before `else`)
   - Call at end of loop body (before increment)
   - Call immediately before `return` statement

2. **Register Pressure:**
   - Many local variables (`v1` through `v10`)
   - Explicit register variables bound to call-clobbered registers
   - Variables used both before and after calls

3. **Control Flow Complexity:**
   - `if-else` statements create basic blocks
   - Loops with calls at the end
   - `switch` statements with multiple cases
   - Different execution paths based on parameters

4. **Anti-Optimization Measures:**
   - `volatile` globals prevent dead code elimination
   - `noinline` and `noclone` attributes preserve function calls
   - Results accumulated and printed

**Compilation and Testing:**
