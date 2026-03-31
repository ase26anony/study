/* test-mcf-fixup-edge.c
 * 
 * This test is designed to trigger the uncovered debug dump lines in
 * GCC's mcf.cc function `dump_fixup_edge`, specifically the branches
 * that print labels for artificial source/sink nodes (NEW_EXIT, NEW_ENTRY).
 *
 * To achieve coverage, compile this with a debug-enabled GCC (configured
 * with --enable-checking) that defines MCF_DEBUG, using optimization
 * level -O2 or higher and targeting a register‑constrained architecture.
 *
 * Example compilation for coverage:
 *   gcc-debug -O2 -march=armv7-a -c test-mcf-fixup-edge.c -o test.o
 *
 * The program's runtime is irrelevant; coverage occurs inside the compiler
 * during the register allocation phase (IRA) when the min‑cost flow solver
 * builds and dumps the fixup graph.
 */

/* Force the high‑pressure function to use the priority‑based IRA algorithm,
 * which employs the min‑cost flow solver. Also target ARMv7‑a to limit
 * available general‑purpose registers, making pressure more acute.
 */
__attribute__((optimize("O2", "-fira-algorithm=priority")))
__attribute__((target("arch=armv7-a")))
void high_pressure_function(void)
{
    /* Declare many integer variables that will become separate pseudo‑registers.
     * Using exactly 16 variables gives a conflict graph of a size that often
     * triggers the addition of artificial source/sink nodes.
     */
    int v0, v1, v2, v3, v4, v5, v6, v7;
    int v8, v9, v10, v11, v12, v13, v14, v15;

    /* Initialize all variables with distinct values to prevent coalescing.
     * The arithmetic uses only the variables themselves, ensuring no
     * immediate operands that could be folded away.
     */
    v0  = 1;
    v1  = v0 + 2;
    v2  = v1 - v0;
    v3  = v2 * v1;
    v4  = v3 ^ v2;
    v5  = v4 | v3;
    v6  = v5 & v4;
    v7  = v6 << 2;
    v8  = v7 >> 1;
    v9  = v8 + v7;
    v10 = v9 - v8;
    v11 = v10 * v9;
    v12 = v11 ^ v10;
    v13 = v12 | v11;
    v14 = v13 & v12;
    v15 = v14 << 1;

    /* Insert a volatile asm that clobbers many ARM registers and memory.
     * This increases perceived register pressure and prevents the compiler
     * from reordering or removing the variable computations.
     */
    asm volatile(""
                 :
                 :
                 : "memory",
                   "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                   "r8", "r9", "r10", "r11", "r12", "r14");

    /* Create a multi‑way conditional block where different subsets of
     * variables are live across edges. This forces the conflict graph to
     * have complex interferences and increases the chance that the
     * min‑cost flow solver will need to insert NEW_ENTRY/NEW_EXIT nodes.
     */
    if (v0 & 1) {
        /* Block A: use v0..v7, making them live across the edge into A. */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v0 - v4;
        v2 = v1 ^ v7;
        /* Ensure all are referenced again before the branch ends. */
        v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7;
    } else if (v8 & 2) {
        /* Block B: use v8..v15, making them live across the edge into B. */
        v9  = v10 + v11;
        v12 = v13 * v14;
        v15 = v8 - v12;
        v10 = v9 ^ v15;
        v8  = v9 + v10 + v11 + v12 + v13 + v14 + v15;
    } else {
        /* Block C: use a mixture of both halves, creating cross‑block liveness. */
        v0  = v8 + v9;
        v1  = v10 - v11;
        v2  = v12 * v13;
        v3  = v14 ^ v15;
        v4  = v0 + v1 + v2 + v3;
        v8  = v4 - v0;
        v9  = v4 - v1;
        v10 = v4 - v2;
        v11 = v4 - v3;
    }

    /* After the conditional, use all variables again to keep them live
     * through the merge point. This maximizes register pressure across
     * the whole control‑flow graph.
     */
    v0  = v0  + v1;
    v2  = v2  + v3;
    v4  = v4  + v5;
    v6  = v6  + v7;
    v8  = v8  + v9;
    v10 = v10 + v11;
    v12 = v12 + v13;
    v14 = v14 + v15;

    /* Final volatile barrier to prevent dead‑code elimination. */
    asm volatile("" ::: "memory");
}

/* A trivial main function to make the file compilable.
 * The interesting coverage happens when GCC compiles
 * high_pressure_function above.
 */
int main(void)
{
    return 0;
}
