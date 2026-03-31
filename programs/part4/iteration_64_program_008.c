/* test-mcf-fixup-edge.c
 * 
 * This test is designed to trigger the uncovered debug dump lines in GCC's
 * mcf.cc function `dump_fixup_edge`, specifically the branches that print
 * labels for artificial source/sink nodes (NEW_EXIT, NEW_ENTRY) added during
 * min-cost flow fixup graph construction.
 *
 * To achieve coverage, compile this with a version of GCC configured with
 * internal debugging enabled (--enable-checking) so that MCF_DEBUG is defined.
 * Example compilation:
 *   gcc-debug -O2 -march=armv7-a -c test-mcf-fixup-edge.c -o test.o
 *
 * The runtime behavior of the resulting binary is irrelevant; coverage occurs
 * during the compiler's register allocation phase.
 */

/* Force the high-pressure function to use the priority-based IRA algorithm,
 * which employs the min-cost flow solver we want to exercise.
 */
#define IRA_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARMv7-A to limit available general-purpose registers, increasing
 * register pressure and forcing the allocator to use the fixup graph.
 */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Mark a function as noinline to prevent merging with main */
#define NOINLINE __attribute__((noinline))

/* Volatile assembly to clobber many ARM registers, increasing perceived
 * register pressure and discouraging trivial allocation.
 */
#define CLOBBER_MANY_REGS \
    asm volatile("" : : : "memory", \
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
                 "r8", "r9", "r10", "r11", "r12", "r14")

/* High-pressure function with many simultaneously live integer variables
 * across complex control flow.
 */
IRA_PRIORITY ARM_TARGET NOINLINE
static void high_pressure_function(void) {
    /* Declare many integer variables to create dense conflict graph.
     * Using exactly 16 variables to influence graph sizing heuristics.
     */
    int v0, v1, v2, v3, v4, v5, v6, v7;
    int v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize all variables with distinct values to prevent dead store
     * elimination. Use arithmetic that cannot be optimized away.
     */
    v0  = 1;
    v1  = v0 + 2;
    v2  = v1 * 3;
    v3  = v2 - v0;
    v4  = v3 ^ v1;
    v5  = v4 | v2;
    v6  = v5 & v3;
    v7  = v6 << 2;
    v8  = v7 >> 1;
    v9  = v8 + v4;
    v10 = v9 - v5;
    v11 = v10 * v6;
    v12 = v11 / (v7 ? v7 : 1);
    v13 = v12 ^ v8;
    v14 = v13 | v9;
    v15 = v14 & v10;
    
    /* Complex conditional chain to create multiple basic blocks with
     * different subsets of variables live across edges.
     */
    CLOBBER_MANY_REGS; /* Force spill/reload points */
    
    if (v0 & 1) {
        /* Block A: use first subset */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 ^ v9;
        CLOBBER_MANY_REGS;
    } else {
        /* Block B: use different subset */
        v10 = v11 + v12;
        v13 = v10 * v14;
        v15 = v13 - v0;
        v1 = v15 ^ v2;
        CLOBBER_MANY_REGS;
    }
    
    /* Second level of conditionals to increase control flow complexity */
    if (v4 > v8) {
        /* Block C: mix variables from both previous branches */
        v3 = v10 + v13;
        v5 = v3 * v15;
        v7 = v5 - v1;
        v9 = v7 ^ v11;
        CLOBBER_MANY_REGS;
    } else {
        /* Block D: another mix */
        v12 = v2 + v6;
        v14 = v12 * v8;
        v0 = v14 - v4;
        v10 = v0 ^ v13;
        CLOBBER_MANY_REGS;
    }
    
    /* Third conditional to create a diamond merge point */
    switch (v9 & 3) {
        case 0:
            v11 = v14 + v15;
            v2  = v11 * v3;
            v6  = v2 - v5;
            break;
        case 1:
            v12 = v0 + v1;
            v4  = v12 * v7;
            v8  = v4 - v10;
            break;
        case 2:
            v13 = v2 + v4;
            v5  = v13 * v6;
            v9  = v5 - v8;
            break;
        default:
            v14 = v3 + v7;
            v10 = v14 * v11;
            v15 = v10 - v12;
            break;
    }
    
    /* Final computation using all variables to ensure they remain live
     * through most of the function, creating maximum interference.
     */
    v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
         v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    /* Volatile use to prevent dead code elimination */
    asm volatile("" : "+r"(v0));
}

/* Main function exists only to make the file compilable.
 * The actual coverage happens during compilation of high_pressure_function.
 */
int main(void) {
    /* Could call the function, but coverage is compile-time only */
    high_pressure_function();
    return 0;
}
