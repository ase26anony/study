/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * when compiled with a GCC built with --enable-checking (MCF_DEBUG defined)
 */

/* Force use of priority-based IRA algorithm */
#define IRA_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited registers (16 GP registers, some reserved) */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Volatile assembly to clobber many registers */
#define CLOBBER_MANY asm volatile("" : : : \
    "memory", \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14" /* lr */)

/* Create high register pressure function */
ARM_TARGET IRA_ATTR
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize with volatile values to prevent constant propagation */
    volatile int seed = 42;
    v0 = seed + 0;
    v1 = seed + 1;
    v2 = seed + 2;
    v3 = seed + 3;
    v4 = seed + 4;
    v5 = seed + 5;
    v6 = seed + 6;
    v7 = seed + 7;
    v8 = seed + 8;
    v9 = seed + 9;
    v10 = seed + 10;
    v11 = seed + 11;
    v12 = seed + 12;
    v13 = seed + 13;
    v14 = seed + 14;
    v15 = seed + 15;
    
    /* Complex control flow to create many basic blocks */
    /* Each branch keeps different subsets of variables live */
    
    /* Block 1: Use first 8 variables */
    CLOBBER_MANY;
    v0 = v1 + v2;
    v3 = v4 * v5;
    v6 = v7 - v0;
    
    /* Conditional branch creating separate flow paths */
    if (v0 > 100) {
        /* Block 2: Use middle 8 variables, keep first 4 live */
        CLOBBER_MANY;
        v8 = v9 + v10;
        v11 = v12 * v13;
        v14 = v15 - v8;
        
        /* Use v0-v3 from parent block to keep them live */
        v8 = v8 + v0 + v1 + v2 + v3;
        
        /* Nested condition */
        if (v8 < 200) {
            /* Block 3: Mix variables from both sets */
            CLOBBER_MANY;
            v4 = v8 + v9;
            v5 = v10 * v11;
            v6 = v12 - v13;
            v7 = v14 + v15;
            
            /* Use all variables in computation */
            v0 = v0 + v4 + v8 + v12;
        } else {
            /* Block 4: Different mix */
            CLOBBER_MANY;
            v1 = v9 + v10;
            v2 = v11 * v12;
            v3 = v13 - v14;
            
            /* Force all variables live */
            v15 = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + 
                  v8 + v9 + v10 + v11 + v12 + v13 + v14;
        }
    } else {
        /* Block 5: Alternative path using different variables */
        CLOBBER_MANY;
        v9 = v10 + v11;
        v12 = v13 * v14;
        v15 = v0 - v9;
        
        /* Keep many variables live across loop */
        for (int i = 0; i < 3; i++) {
            CLOBBER_MANY;
            v0 = v0 + v9;
            v1 = v1 + v10;
            v2 = v2 + v11;
            v3 = v3 + v12;
            v4 = v4 + v13;
            v5 = v5 + v14;
            v6 = v6 + v15;
            v7 = v7 + i;
            
            /* Small switch to create more blocks */
            switch (i) {
                case 0:
                    v8 = v0 * v1;
                    break;
                case 1:
                    v9 = v2 * v3;
                    break;
                case 2:
                    v10 = v4 * v5;
                    break;
            }
        }
    }
    
    /* Final block: Use all variables one more time */
    CLOBBER_MANY;
    volatile int result = 
        v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 +
        v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
}

/* Second function with different pressure pattern */
ARM_TARGET IRA_ATTR
void another_high_pressure_func(int cond) {
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    volatile int init = cond;
    a0 = init + 0;  a1 = init + 1;  a2 = init + 2;  a3 = init + 3;
    a4 = init + 4;  a5 = init + 5;  a6 = init + 6;  a7 = init + 7;
    a8 = init + 8;  a9 = init + 9;  a10 = init + 10; a11 = init + 11;
    a12 = init + 12; a13 = init + 13; a14 = init + 14;
    
    /* Large switch statement creates many basic blocks */
    switch (cond & 7) {
        case 0:
            a0 = a1 + a2; a3 = a4 * a5; a6 = a7 - a8;
            /* Keep others live */
            a9 = a10 + a11 + a12 + a13 + a14;
            break;
        case 1:
            a1 = a2 + a3; a4 = a5 * a6; a7 = a8 - a9;
            a10 = a11 + a12 + a13 + a14 + a0;
            break;
        case 2:
            a2 = a3 + a4; a5 = a6 * a7; a8 = a9 - a10;
            a11 = a12 + a13 + a14 + a0 + a1;
            break;
        case 3:
            a3 = a4 + a5; a6 = a7 * a8; a9 = a10 - a11;
            a12 = a13 + a14 + a0 + a1 + a2;
            break;
        case 4:
            a4 = a5 + a6; a7 = a8 * a9; a10 = a11 - a12;
            a13 = a14 + a0 + a1 + a2 + a3;
            break;
        case 5:
            a5 = a6 + a7; a8 = a9 * a10; a11 = a12 - a13;
            a14 = a0 + a1 + a2 + a3 + a4;
            break;
        case 6:
            a6 = a7 + a8; a9 = a10 * a11; a12 = a13 - a14;
            a0 = a1 + a2 + a3 + a4 + a5;
            break;
        default:
            a7 = a8 + a9; a10 = a11 * a12; a13 = a14 - a0;
            a1 = a2 + a3 + a4 + a5 + a6;
    }
    
    /* Force all live at end */
    volatile int sum = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + 
                      a10 + a11 + a12 + a13 + a14;
    asm volatile("" : : "r"(sum) : "memory");
}

/* Main exists only to make the file compilable */
int main(void) {
    high_pressure_function();
    another_high_pressure_func(3);
    return 0;
}
