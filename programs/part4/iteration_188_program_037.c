**Key design elements that target the uncovered code:**

1. **Multiple live values across calls**: The function declares 15+ variables of different types (int, long, float, double) that are used both before and after function calls.

2. **Non-inline function calls**: Both `helper_function` and `helper_function2` are marked `noinline` to ensure actual call instructions are generated.

3. **Instruction placement opportunities**: The `intermediate1` and `intermediate2` variables are computed before calls but used after calls, giving the compiler flexibility in scheduling these computations around the save/restore sequences.

4. **Memory barriers**: `asm volatile("" ::: "memory")` prevents reordering and forces the compiler to treat values as potentially modified.

5. **Loop structure**: The loop in `main` causes repeated execution of the caller-save sequence, increasing the chance of triggering the specific code path.

6. **Volatile usage**: Volatile variables prevent dead code elimination while maintaining data dependencies.

7. **Mixed computations**: Integer and floating-point operations engage different register files, increasing overall register pressure.

**Compilation and testing:**
