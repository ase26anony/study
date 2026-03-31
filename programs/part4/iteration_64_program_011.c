/* test_mcf_coverage.c
 * Designed to trigger min-cost flow fixup graph debug output
 * in GCC's IRA register allocator when compiled with MCF_DEBUG.
 */

/* Force use of priority-based IRA algorithm */
#define IRA_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited registers */
#define ARM_ATTR __attribute__((target("arch=armv7-a")))

/* Combine attributes */
#define HIGH_PRESSURE IRA_ATTR ARM_ATTR

/* Volatile assembly to clobber ARM registers */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "memory")

/* Create high register pressure with many live ranges */
HIGH_PRESSURE void high_pressure_func(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create pseudo-registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with non-constant values */
    v1 = cond1;
    v2 = cond2 + 1;
    v3 = cond3 * 2;
    v4 = v1 + v2;
    v5 = v2 - v3;
    v6 = v3 * v4;
    v7 = v4 / (cond1 ? 2 : 3);
    v8 = v5 ^ v6;
    v9 = v6 | v7;
    v10 = v7 & v8;
    v11 = v8 + v9;
    v12 = v9 - v10;
    v13 = v10 * v11;
    v14 = v11 ^ v12;
    v15 = v12 | v13;
    v16 = v13 & v14;
    
    /* Force complex control flow with many live variables */
    if (cond1 > 0) {
        /* Use all variables in block 1 */
        CLOBBER_REGS;
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        v10 = v11 ^ v12;
        v13 = v14 | v15;
        v16 = v1 + v4;
        
        if (cond2 > 0) {
            /* Nested block with different variable usage */
            CLOBBER_REGS;
            v2 = v3 + v4;
            v5 = v6 * v7;
            v8 = v9 - v10;
            v11 = v12 ^ v13;
            v14 = v15 | v16;
            v1 = v2 + v5;
        } else {
            /* Alternative path */
            CLOBBER_REGS;
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 ^ v14;
            v15 = v16 | v1;
            v2 = v3 + v6;
        }
        
        /* Merge point - use many variables */
        CLOBBER_REGS;
        v4 = v1 + v2 + v3;
        v8 = v5 + v6 + v7;
        v12 = v9 + v10 + v11;
        v16 = v13 + v14 + v15;
    } else {
        /* Else branch with different variable patterns */
        CLOBBER_REGS;
        v1 = v16 - v15;
        v2 = v15 - v14;
        v3 = v14 - v13;
        v4 = v13 - v12;
        v5 = v12 - v11;
        v6 = v11 - v10;
        v7 = v10 - v9;
        v8 = v9 - v8;
        
        /* Switch to create more control flow complexity */
        switch (cond3 & 3) {
            case 0:
                v9 = v1 * v2;
                v10 = v3 * v4;
                v11 = v5 * v6;
                v12 = v7 * v8;
                break;
            case 1:
                v9 = v1 + v2;
                v10 = v3 + v4;
                v11 = v5 + v6;
                v12 = v7 + v8;
                break;
            case 2:
                v9 = v1 ^ v2;
                v10 = v3 ^ v4;
                v11 = v5 ^ v6;
                v12 = v7 ^ v8;
                break;
            default:
                v9 = v1 | v2;
                v10 = v3 | v4;
                v11 = v5 | v6;
                v12 = v7 | v8;
                break;
        }
        
        CLOBBER_REGS;
        v13 = v9 + v10;
        v14 = v11 + v12;
        v15 = v13 * v14;
        v16 = v15 ^ (v1 + v2 + v3);
    }
    
    /* Final computation using all variables to ensure liveness */
    CLOBBER_REGS;
    int result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
        v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+r" (result));
}

/* Second high-pressure function with loop to increase graph complexity */
HIGH_PRESSURE void high_pressure_loop(int iterations) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    a1 = iterations;
    a2 = a1 + 1;
    a3 = a2 * 2;
    a4 = a3 - a1;
    a5 = a4 ^ a2;
    a6 = a5 | a3;
    a7 = a6 & a4;
    a8 = a7 + a5;
    a9 = a8 - a6;
    a10 = a9 * a7;
    a11 = a10 ^ a8;
    a12 = a11 | a9;
    a13 = a12 & a10;
    a14 = a13 + a11;
    
    /* Loop with many live variables */
    for (int i = 0; i < iterations && i < 10; i++) {
        CLOBBER_REGS;
        
        /* Complex loop body with conditional updates */
        if (i & 1) {
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
            a10 = a11 ^ a12;
            a13 = a14 + a1;
        } else {
            a2 = a3 + a4;
            a5 = a6 * a7;
            a8 = a9 - a10;
            a11 = a12 ^ a13;
            a14 = a1 + a2;
        }
        
        /* Cross dependencies */
        a3 = a1 + a14;
        a6 = a4 + a13;
        a9 = a7 + a12;
        a12 = a10 + a11;
    }
    
    /* Force all variables to be live at exit */
    CLOBBER_REGS;
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + 
              a9 + a10 + a11 + a12 + a13 + a14;
    asm volatile("" : "+r" (sum));
}

/* Main exists only to make the file compilable */
int main() {
    high_pressure_func(1, 2, 3);
    high_pressure_loop(5);
    return 0;
}
