**Key design elements that target the uncovered code:**

1. **High Register Pressure**: The `worker_function` declares 20+ variables of different types (int, long, float, double) that are all used in computations both before and after the function call.

2. **Live Values Across Call**: Variables `r1`, `r2`, `r3`, `r4` are computed before the call and used after the call, forcing the compiler to preserve them across the call.

3. **Instruction Movement Opportunity**: The `temp = helper_func2(result3, f1)` computation uses both pre-call (`f1`) and post-call (`result3`) values, creating a scheduling dilemma that might cause GCC to move instructions around the save/restore sequence.

4. **Memory Barriers**: `asm volatile("" ::: "memory")` prevents the compiler from reordering instructions across the barrier, encouraging explicit save/restore placement.

5. **Non-Inline Calls**: Both `helper_func` and `helper_func2` are marked `noinline`, ensuring actual call instructions are generated.

6. **Loop Structure**: The main loop calls `worker_function` 100 times, creating repeated opportunities for caller-save decisions.

7. **Basic Block Structure**: The `if (iteration % 10 == 0)` check after the call in `main` ensures the call isn't at the end of its basic block.

**Compilation and testing:**
