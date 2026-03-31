/* test-mcf-dump-fixup-edge.c
 * 
 * This test is designed to trigger the uncovered debug dump lines in
 * GCC's mcf.cc (Min-Cost Flow solver) when compiled with a GCC built
 * with internal checking enabled (--enable-checking).
 *
 * The uncovered lines are in `dump_fixup_edge` and print special node
 * labels like "NEW_EXIT" and "NEW_ENTRY". To reach them, we must:
 *   1. Create a function with very high register pressure.
 *   2. Force IRA to use the priority-based allocator (which uses MCF).
 *   3. Target a register‑scarce architecture (ARM).
 *   4. Use volatile assembly to clobber many registers, increasing
 *      perceived pressure.
 *   5. Structure the code so that many integer variables are live
 *      across complex control flow, forcing IRA to build a fixup graph
 *      that requires artificial source/sink nodes.
 *
 * Coverage is achieved at compile‑time when GCC (with MCF_DEBUG) runs
 * the integrated register allocator on this function.
 */

/* Force the priority‑based IRA algorithm for this function.
 * This is the allocator that uses the min‑cost flow solver.
 */
void __attribute__((optimize("O2", "-fira-algorithm=priority")))
/* Target ARMv7‑a, which has few general‑purpose registers,
 * making register pressure more acute.
 */
__attribute__((target("arch=armv7-a")))
high_pressure_function(void)
{
    /* Declare many integer variables that will become pseudo‑registers.
     * We use exactly 16 variables to influence the conflict graph size.
     */
    int v0, v1, v2, v3, v4, v5, v6, v7;
    int v8, v9, v10, v11, v12, v13, v14, v15;

    /* Initialize all variables with distinct values to ensure they are
     * all live at the entry of the first conditional block.
     */
    v0  = 0;
    v1  = 1;
    v2  = 2;
    v3  = 3;
    v4  = 4;
    v5  = 5;
    v6  = 6;
    v7  = 7;
    v8  = 8;
    v9  = 9;
    v10 = 10;
    v11 = 11;
    v12 = 12;
    v13 = 13;
    v14 = 14;
    v15 = 15;

    /* Volatile assembly that clobbers many ARM registers and memory.
     * This increases the compiler's perception of register pressure
     * and discourages trivial allocation.
     */
    asm volatile(""
                 :
                 :
                 : "memory",
                   "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                   "r8", "r9", "r10", "r12");

    /* Complex conditional chain that makes different subsets of
     * variables live across block boundaries.
     * Each branch uses a different set of variables in computations,
     * ensuring they remain live through the control‑flow merge points.
     */
    if (v0 & 1) {
        /* Use group 0 */
        v1  = v2 + v3;
        v4  = v5 * v6;
        v7  = v8 - v9;
        v10 = v11 ^ v12;
        v13 = v14 | v15;
        /* Another clobber to split live ranges */
        asm volatile("" ::: "memory");
    } else {
        /* Use group 1 */
        v2  = v3 + v4;
        v5  = v6 * v7;
        v8  = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v0;
        asm volatile("" ::: "memory");
    }

    /* Second conditional level, using yet another mix of variables. */
    if (v1 & 2) {
        v3  = v4 + v5;
        v6  = v7 * v8;
        v9  = v10 - v11;
        v12 = v13 ^ v14;
        v15 = v0 | v1;
        asm volatile("" ::: "memory");
    } else {
        v4  = v5 + v6;
        v7  = v8 * v9;
        v10 = v11 - v12;
        v13 = v14 ^ v15;
        v0  = v1 | v2;
        asm volatile("" ::: "memory");
    }

    /* Third conditional level, ensuring all 16 variables are still live. */
    if (v2 & 4) {
        v5  = v6 + v7;
        v8  = v9 * v10;
        v11 = v12 - v13;
        v14 = v15 ^ v0;
        v1  = v2 | v3;
        asm volatile("" ::: "memory");
    } else {
        v6  = v7 + v8;
        v9  = v10 * v11;
        v12 = v13 - v14;
        v15 = v0 ^ v1;
        v2  = v3 | v4;
        asm volatile("" ::: "memory");
    }

    /* Final use of all variables in a single expression chain.
     * This guarantees they are all live at the exit of the last block.
     */
    v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 +
         v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;

    /* Prevent dead‑code elimination */
    asm volatile("" : "+r"(v0));
}

/* A trivial main function to make the file compilable.
 * The interesting coverage happens during compilation of
 * high_pressure_function, not at runtime.
 */
int main(void)
{
    return 0;
}
