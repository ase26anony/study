**Key Design Elements:**

1. **High Register Pressure:**
   - 12 volatile local variables in each loop iteration
   - Multiple intermediate computations creating additional live values
   - Function calls with 6-8 arguments forcing use of multiple registers

2. **Inline Assembly Clobbering:**
   - Explicitly clobbers all major x86-64 caller-saved registers
   - Clobbers both integer and XMM registers
   - Placed between computations to force save/restore insertion points

3. **Basic Block Structure:**
   - Multiple operations before and after each call/clobber
   - The `helper3` call followed by result accumulation creates a scenario where the insertion might need to update `BB_END`
   - Loop structure ensures instructions are in the middle of basic blocks

4. **Anti-Optimization Measures:**
   - `volatile` variables prevent dead store elimination
   - `noinline, noipa` attributes on helper functions
   - Result accumulation prevents loop elimination
   - Final function call at the end creates additional insertion opportunities

**Compilation and Verification:**
