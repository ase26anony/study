**Key design elements that target the uncovered code:**

1. **High Register Pressure**: The `worker_function` declares 20+ variables of mixed types (int, long, float, double), ensuring they can't all fit in caller-saved registers across the function call.

2. **Live Values Across Calls**: Variables are computed before `helper_function()` calls and used after, creating true liveness that requires save/restore.

3. **Instruction Placement Opportunities**: The mix of computations before and after calls, combined with `asm volatile` memory clobbers, gives the compiler flexibility to move instructions into the save/restore sequence.

4. **Basic Block Structure**: Each call is followed by additional computations, ensuring the call isn't at the end of its basic block.

5. **Loop Context**: The loop in `main()` causes repeated execution, increasing the chance of triggering the specific instruction movement pattern.

**Compilation and testing:**
