/* test-mcf-dump-fixup-edge.c
 * 
 * This test is designed to trigger the uncovered debug dump lines in GCC's
 * mcf.cc, specifically in `dump_fixup_edge`, when compiled with a GCC
 * configured with internal checking (--enable-checking) which defines
 * MCF_DEBUG.
 *
 * The program creates a function with extreme register pressure across
 * complex control flow, forcing IRA's min-cost flow solver to construct
 * a fixup graph with artificial source/sink nodes (new_entry_index,
 * new_exit_index). When MCF_DEBUG is active, this should trigger calls
 * to dump_fixup_edge with those special node indices.
 *
 * Compile with a debug-built GCC (e.g., gcc-debug) using:
 *   gcc-debug -O2 -march=armv7-a -c test-mcf-dump-fixup-edge.c -o test.o
 * or
 *   gcc-debug -O3 -funroll-loops -c test-mcf-dump-fixup-edge.c -o test.o
 *
 * Coverage is achieved at compile-time within GCC's IRA phase, not at runtime.
 */

/* Force use of priority-based IRA algorithm for this function */
#define PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARMv7-a to limit available general-purpose registers */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Function with extreme register pressure */
PRIORITY_IRA ARM_TARGET
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize all variables with distinct values to prevent optimization */
    v0 = 0;
    v1 = 1;
    v2 = 2;
    v3 = 3;
    v4 = 4;
    v5 = 5;
    v6 = 6;
    v7 = 7;
    v8 = 8;
    v9 = 9;
    v10 = 10;
    v11 = 11;
    v12 = 12;
    v13 = 13;
    v14 = 14;
    v15 = 15;
    
    /* Complex control flow with multiple basic blocks */
    /* Each block uses a different subset of variables, keeping many live */
    
    /* Block 1: Use first subset */
    asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12");
    v0 = v1 + v2;
    v3 = v4 * v5;
    v6 = v7 - v8;
    
    if (v0 > 0) {
        /* Block 2: Different subset, but previous variables still live */
        asm volatile("" : : : "memory");
        v9 = v10 + v11;
        v12 = v13 * v14;
        v15 = v0 + v3;  /* Use vars from previous block */
        
        /* Nested condition for more control flow */
        if (v9 < 100) {
            /* Block 3: Mix variables from both previous blocks */
            asm volatile("" : : : "memory");
            v1 = v2 + v12;
            v4 = v5 * v15;
            v7 = v8 - v9;
            v10 = v11 + v1;
        } else {
            /* Block 4: Alternative path */
            asm volatile("" : : : "memory");
            v2 = v3 + v13;
            v5 = v6 * v14;
            v8 = v9 - v10;
            v11 = v12 + v2;
        }
        
        /* Block 5: Use all variables in complex computation */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12");
        v0 = v1 + v2 + v3 + v4;
        v5 = v6 * v7 * v8 * v9;
        v10 = v11 - v12 - v13 - v14;
        v15 = v0 * v5 + v10;
    } else {
        /* Block 6: Alternative main path */
        asm volatile("" : : : "memory");
        v1 = v2 * v3;
        v4 = v5 + v6;
        v7 = v8 * v9;
        v10 = v11 + v12;
        v13 = v14 * v15;
        
        /* Switch-like structure for additional control flow */
        switch (v1 & 3) {
            case 0:
                v0 = v4 + v7;
                v2 = v10 * v13;
                break;
            case 1:
                v3 = v5 - v8;
                v6 = v11 / (v14 ? v14 : 1);
                break;
            case 2:
                v9 = v12 * v15;
                v0 = v1 + v2;
                break;
            default:
                v4 = v7 * v10;
                v13 = v3 - v6;
                break;
        }
    }
    
    /* Final block: Use all variables one more time to extend liveness */
    asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12");
    v0 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    v8 = v8 * v9 * v10 * v11 * v12 * v13 * v14 * v15;
    
    /* Force all variables to be observable (prevent dead code elimination) */
    asm volatile("" : : "r"(v0), "r"(v8) : "memory");
}

/* Trivial main function to make the file compilable */
int main(void) {
    return 0;
}
