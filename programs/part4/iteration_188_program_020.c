**Key design elements that target the uncovered code:**

1. **Multiple live values across calls**: The `worker_function` declares 12 variables of mixed types (int, long, float, double) and performs computations both before and after the `helper_function` call, ensuring they must survive the call.

2. **Instruction movement opportunities**: The `temp = v1 * v2` and `temp = temp - v3` computations in `worker_function`, and similar patterns in `process_values`, create instructions whose results are used after the call but could potentially be scheduled into the save/restore sequence.

3. **Memory barriers**: `asm volatile("" ::: "memory")` statements prevent the compiler from reordering instructions across these barriers, increasing the likelihood of explicit save/restore code generation.

4. **Loop with calls**: The loops in `process_values` and `main` cause repeated execution of call sequences, increasing the probability of triggering the specific code path during coverage testing.

5. **Non-inline functions**: Both `helper_function` and `worker_function` are marked `noinline` to ensure actual call instructions are generated.

6. **Volatile variables**: The `global_counter` and local `acc` variables prevent optimization and ensure side effects are preserved.

**Compilation and execution:**
