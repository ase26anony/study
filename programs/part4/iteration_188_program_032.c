**Key design elements that target the uncovered code:**

1. **High Register Pressure**: 13 variables of mixed types (int, long, float, double) are declared and used both before and after the function call, forcing the compiler to spill registers.

2. **Critical Value Across Call**: The `critical_value` variable is computed before the call and used after, making it a candidate for having its computation instruction moved into the save/restore sequence.

3. **Non-Inline Functions**: Both `helper_function` and `helper_float` are marked `noinline`, ensuring actual call instructions are generated.

4. **Memory Barriers**: `asm volatile("" ::: "memory")` prevents the compiler from reordering instructions across the barriers, making it more likely to generate explicit save/restore sequences.

5. **Loop Structure**: The loop in `main` causes repeated execution of the caller-save logic, increasing the chance of triggering the uncovered code paths.

6. **Basic Block Structure**: Instructions after the function call (using `critical_value` and computing `final_result`) ensure the call isn't at the end of its basic block, which is necessary for the `BB_END` update logic.

7. **Volatile Variables**: Used for `global_counter`, `final_result`, and `accumulator` to prevent optimization and ensure side effects are preserved.

**Compilation and testing:**
