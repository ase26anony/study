/* test-mcf-dump-fixup-edge.c
 * 
 * This test is designed to trigger the uncovered debug dump lines in GCC's
 * mcf.cc, specifically in dump_fixup_edge, when compiled with a GCC built
 * with internal checking enabled (--enable-checking) which defines MCF_DEBUG.
 *
 * The program creates a function with high register pressure and complex
 * control flow to force IRA's min-cost flow solver to create artificial
 * source/sink nodes (new_entry_index, new_exit_index) and potentially dump
 * the fixup graph during compilation.
 *
 * Coverage is achieved at compile-time, not runtime.
 */

/* Force the priority register allocator, which uses the min-cost flow solver */
#define USE_PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARMv7-a to limit available general-purpose registers */
#define TARGET_ARM __attribute__((target("arch=armv7-a")))

/* Volatile assembly to clobber many ARM registers, increasing pressure */
#define CLOBBER_MANY_REGS \
    asm volatile("" : : : "memory", \
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
        "r8", "r9", "r10", "r11", "r12", "r14")

/* Main high-pressure function */
USE_PRIORITY_IRA TARGET_ARM
void high_pressure_function(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize with input parameters and constants to create dependencies */
    v1 = cond1;
    v2 = cond2;
    v3 = cond3;
    v4 = v1 + v2;
    v5 = v2 * v3;
    v6 = v3 - v1;
    v7 = v4 + v5;
    v8 = v5 - v6;
    v9 = v6 * v7;
    v10 = v7 + v8;
    v11 = v8 - v9;
    v12 = v9 * v10;
    v13 = v10 + v11;
    v14 = v11 - v12;
    v15 = v12 * v13;
    
    /* Clobber registers to force spills */
    CLOBBER_MANY_REGS;
    
    /* Complex control flow with multiple basic blocks */
    if (cond1 > 0) {
        /* Use all variables in block 1 to keep them live */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 + v9;
        v10 = v8 * v11;
        v12 = v10 - v13;
        v14 = v12 + v15;
        v3 = v14 * v2;
        CLOBBER_MANY_REGS;
    } else {
        /* Different computation in block 2, still using all variables */
        v2 = v3 + v4;
        v5 = v2 * v6;
        v7 = v5 - v8;
        v9 = v7 + v10;
        v11 = v9 * v12;
        v13 = v11 - v14;
        v15 = v13 + v1;
        v4 = v15 * v3;
        CLOBBER_MANY_REGS;
    }
    
    /* Another level of conditional nesting */
    if (cond2 < 0) {
        /* Block 3 */
        v5 = v6 + v7;
        v8 = v5 * v9;
        v10 = v8 - v11;
        v12 = v10 + v13;
        v14 = v12 * v15;
        v1 = v14 - v2;
        v3 = v1 + v4;
        CLOBBER_MANY_REGS;
        
        /* Inner switch-like structure */
        switch (cond3 & 3) {
            case 0:
                v6 = v7 * v8;
                v9 = v6 + v10;
                break;
            case 1:
                v7 = v8 - v9;
                v10 = v7 * v11;
                break;
            case 2:
                v8 = v9 + v10;
                v11 = v8 - v12;
                break;
            default:
                v9 = v10 * v11;
                v12 = v9 + v13;
                break;
        }
    } else {
        /* Block 4 */
        v6 = v7 - v8;
        v9 = v6 * v10;
        v11 = v9 + v12;
        v13 = v11 - v14;
        v15 = v13 * v1;
        v2 = v15 + v3;
        v4 = v2 - v5;
        CLOBBER_MANY_REGS;
    }
    
    /* Final computations using all variables to ensure liveness overlaps */
    v1 = v1 + v2 + v3 + v4 + v5;
    v6 = v6 - v7 - v8 - v9 - v10;
    v11 = v11 * v12 * v13 * v14 * v15;
    
    /* Use results to prevent dead code elimination */
    asm volatile("" : : "r"(v1), "r"(v6), "r"(v11));
}

/* Secondary function with different pressure pattern */
USE_PRIORITY_IRA TARGET_ARM
void another_high_pressure_function(int x) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    a1 = x;
    a2 = a1 * 2;
    a3 = a2 + 1;
    a4 = a3 - x;
    a5 = a4 * a1;
    a6 = a5 + a2;
    a7 = a6 - a3;
    a8 = a7 * a4;
    a9 = a8 + a5;
    a10 = a9 - a6;
    a11 = a10 * a7;
    a12 = a11 + a8;
    a13 = a12 - a9;
    a14 = a13 * a10;
    
    CLOBBER_MANY_REGS;
    
    /* Loop to increase complexity */
    for (int i = 0; i < 3; i++) {
        if (i & 1) {
            a1 = a2 + a3;
            a4 = a5 - a6;
            a7 = a8 * a9;
            a10 = a11 + a12;
            a13 = a14 - a1;
        } else {
            a2 = a3 - a4;
            a5 = a6 * a7;
            a8 = a9 + a10;
            a11 = a12 - a13;
            a14 = a1 * a2;
        }
        CLOBBER_MANY_REGS;
    }
    
    /* Force use of all variables */
    asm volatile("" : : 
        "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
        "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
        "r"(a11), "r"(a12), "r"(a13), "r"(a14));
}

/* Trivial main to make the file compilable */
int main() {
    return 0;
}
