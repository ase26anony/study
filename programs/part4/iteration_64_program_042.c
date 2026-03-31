/* test-mcf-fixup-edge.c
 * 
 * This test is designed to trigger the uncovered lines in GCC's mcf.cc
 * within the function dump_fixup_edge, specifically the code that prints
 * labels for special node indices (NEW_EXIT, NEW_ENTRY, etc.).
 *
 * The test creates a function with extremely high register pressure
 * across complex control flow to force IRA's min-cost flow solver to
 * construct a fixup graph with artificial source/sink nodes.
 *
 * Compile with a debug-built GCC (configured with --enable-checking)
 * to enable MCF_DEBUG and trigger the debug dump paths:
 *
 *   gcc-debug -O2 -c test-mcf-fixup-edge.c -o test.o
 *
 * For ARM target (fewer registers, more pressure):
 *   gcc-debug -O2 -march=armv7-a -mtune=cortex-a8 -c test-mcf-fixup-edge.c
 *
 * The actual execution of the program is irrelevant; coverage occurs
 * during compilation within GCC's IRA phase.
 */

/* Force priority-based register allocation algorithm for this function,
 * which uses the min-cost flow solver we want to exercise.
 */
#ifdef __GNUC__
#define PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define PRIORITY_IRA
#endif

/* Target ARM architecture to limit available registers */
#ifdef __GNUC__
#define ARM_TARGET __attribute__((target("arch=armv7-a")))
#else
#define ARM_TARGET
#endif

/* Function with extreme register pressure */
PRIORITY_IRA ARM_TARGET
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with different values to prevent optimization */
    v1 = 1;   v2 = 2;   v3 = 3;   v4 = 4;
    v5 = 5;   v6 = 6;   v7 = 7;   v8 = 8;
    v9 = 9;   v10 = 10; v11 = 11; v12 = 12;
    v13 = 13; v14 = 14; v15 = 15; v16 = 16;
    
    /* Volatile assembly to clobber many registers and prevent optimization.
     * For ARM, we clobber r0-r12 to increase perceived register pressure.
     */
    asm volatile("" : : : "memory",
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "r12");
    
    /* Complex control flow with many live variables across blocks */
    /* Block 1: Use all variables in computations */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v10 = v11 / 2;
    v12 = v13 | v14;
    v15 = v16 & v1;
    
    /* Conditional branch creating multiple paths */
    if (v1 > v2) {
        /* Block 2: Different subset of variables live */
        v3 = v4 + v5;
        v6 = v7 * v8;
        v9 = v10 - v11;
        v12 = v13 | v14;
        v15 = v16 ^ v1;
        
        /* Nested condition for more control flow complexity */
        if (v3 < v4) {
            v5 = v6 + v7;
            v8 = v9 * v10;
            v11 = v12 - v13;
            v14 = v15 | v16;
        } else {
            v5 = v6 - v7;
            v8 = v9 / v10;
            v11 = v12 | v13;
            v14 = v15 ^ v16;
        }
        
        /* Use results to keep variables live */
        v1 = v3 + v5;
        v2 = v8 - v11;
    } else {
        /* Block 3: Alternative path with different variable usage */
        v4 = v6 + v8;
        v10 = v12 * v14;
        v3 = v5 - v7;
        v9 = v11 | v13;
        v15 = v1 ^ v2;
        
        /* Another conditional inside else branch */
        if (v4 > v10) {
            v6 = v8 + v10;
            v12 = v14 * v3;
            v5 = v7 - v9;
            v11 = v13 | v15;
        }
        
        v1 = v4 + v6;
        v2 = v10 - v12;
    }
    
    /* Block 4: Merge point - use all variables again */
    v3 = v1 + v2;
    v4 = v5 + v6;
    v7 = v8 + v9;
    v10 = v11 + v12;
    v13 = v14 + v15;
    v16 = v3 + v4 + v7 + v10 + v13;
    
    /* Another volatile asm to prevent dead code elimination */
    asm volatile("" : : : "memory",
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
        "r8", "r9", "r10", "r11", "r12");
    
    /* Final use of all variables to ensure they're live until the end */
    v1 = v16 - v15;
    v2 = v14 - v13;
    v3 = v12 - v11;
    v4 = v10 - v9;
    v5 = v8 - v7;
    v6 = v4 + v5;
    v7 = v2 + v3;
    v8 = v1 + v6 + v7;
    
    /* Result must be used to prevent optimization */
    asm volatile("" : "+r"(v8) : : "memory");
}

/* Additional high-pressure function with switch statement for more
 * complex control flow graph */
PRIORITY_IRA ARM_TARGET
void high_pressure_switch_function(int x) {
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5, a6 = 6;
    int b1 = 7, b2 = 8, b3 = 9, b4 = 10, b5 = 11, b6 = 12;
    
    /* Clobber registers */
    asm volatile("" : : : "memory",
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
    
    /* Switch creates multiple basic blocks */
    switch (x) {
        case 0:
            a1 = a2 + a3;
            b1 = b2 - b3;
            a4 = a5 * a6;
            b4 = b5 | b6;
            break;
        case 1:
            a2 = a3 + a4;
            b2 = b3 - b4;
            a5 = a6 * a1;
            b5 = b6 | b1;
            break;
        case 2:
            a3 = a4 + a5;
            b3 = b4 - b5;
            a6 = a1 * a2;
            b6 = b1 | b2;
            break;
        case 3:
            a4 = a5 + a6;
            b4 = b5 - b6;
            a1 = a2 * a3;
            b1 = b2 | b3;
            break;
        default:
            a5 = a6 + a1;
            b5 = b6 - b1;
            a2 = a3 * a4;
            b2 = b3 | b4;
            break;
    }
    
    /* Use all variables after switch */
    int result = a1 + a2 + a3 + a4 + a5 + a6 +
                 b1 + b2 + b3 + b4 + b5 + b6;
    
    asm volatile("" : "+r"(result) : : "memory");
}

/* Main function exists only to make the file compilable */
int main(void) {
    /* Call high-pressure functions to ensure they're not eliminated */
    high_pressure_function();
    high_pressure_switch_function(0);
    return 0;
}
