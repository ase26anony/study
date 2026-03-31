/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * when compiled with a GCC built with --enable-checking (MCF_DEBUG defined)
 */

/* Force use of priority-based IRA algorithm which uses min-cost flow solver */
#define OPT_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority"), \
                               target("arch=armv7-a", "no-thumb")))

/* Volatile assembly to clobber many ARM registers */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "memory")

/* Function with extreme register pressure */
OPT_ATTR
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with different values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5; v6 = 6; v7 = 7; v8 = 8;
    v9 = 9; v10 = 10; v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15; v16 = 16;
    
    /* Complex control flow to create many basic blocks */
    /* Each branch uses different subsets of variables to create complex liveness */
    
    CLOBBER_REGS; /* Force compiler to assume registers are modified */
    
    /* First level of branching */
    if (v1 > 0) {
        /* Use subset 1 */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        CLOBBER_REGS;
        
        /* Nested branching */
        if (v2 < 100) {
            /* Use subset 2 */
            v11 = v12 + v13;
            v14 = v15 * v16;
            v1 = v2 + v3;
            CLOBBER_REGS;
            
            /* More computations to extend live ranges */
            v4 = v5 + v6;
            v7 = v8 * v9;
            v10 = v11 - v12;
        } else {
            /* Use subset 3 */
            v13 = v14 + v15;
            v16 = v1 * v2;
            v3 = v4 + v5;
            CLOBBER_REGS;
            
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 + v14;
        }
        
        /* Common code with different variable usage */
        v15 = v16 + v1;
        v2 = v3 * v4;
    } else {
        /* Alternative path with different variable usage */
        v5 = v6 + v7;
        v8 = v9 * v10;
        v11 = v12 - v13;
        CLOBBER_REGS;
        
        if (v5 > 50) {
            v14 = v15 + v16;
            v1 = v2 * v3;
            v4 = v5 + v6;
            CLOBBER_REGS;
            
            v7 = v8 * v9;
            v10 = v11 - v12;
            v13 = v14 + v15;
        } else {
            v16 = v1 + v2;
            v3 = v4 * v5;
            v6 = v7 + v8;
            CLOBBER_REGS;
            
            v9 = v10 * v11;
            v12 = v13 - v14;
            v15 = v16 + v1;
        }
        
        v2 = v3 * v4;
        v5 = v6 + v7;
    }
    
    /* Final complex switch statement to create more control flow */
    switch (v1 & 0x7) {
        case 0:
            v8 = v9 + v10;
            v11 = v12 * v13;
            v14 = v15 - v16;
            break;
        case 1:
            v2 = v3 + v4;
            v5 = v6 * v7;
            v8 = v9 - v10;
            break;
        case 2:
            v11 = v12 + v13;
            v14 = v15 * v16;
            v1 = v2 + v3;
            break;
        case 3:
            v4 = v5 + v6;
            v7 = v8 * v9;
            v10 = v11 - v12;
            break;
        case 4:
            v13 = v14 + v15;
            v16 = v1 * v2;
            v3 = v4 + v5;
            break;
        case 5:
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 + v14;
            break;
        case 6:
            v15 = v16 + v1;
            v2 = v3 * v4;
            v5 = v6 + v7;
            break;
        case 7:
            v8 = v9 * v10;
            v11 = v12 - v13;
            v14 = v15 + v16;
            break;
    }
    
    /* Force all variables to be used at the end to maintain liveness */
    v1 = v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(v1) : "memory");
}

/* Another function with different pressure pattern */
OPT_ATTR
void another_high_pressure_function(int param) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize from parameter to create dependencies */
    a1 = param; a2 = param + 1; a3 = param + 2; a4 = param + 3;
    a5 = param + 4; a6 = param + 5; a7 = param + 6; a8 = param + 7;
    a9 = param + 8; a10 = param + 9; a11 = param + 10; a12 = param + 11;
    a13 = param + 12; a14 = param + 13;
    
    /* Loop with high pressure to create many pseudo registers */
    for (int i = 0; i < 10; i++) {
        /* Rotate values through variables */
        int t = a1;
        a1 = a2 + a3; a2 = a4 * a5; a3 = a6 - a7;
        a4 = a8 + a9; a5 = a10 * a11; a6 = a12 - a13;
        a7 = a14 + t; a8 = a1 * a2; a9 = a3 - a4;
        a10 = a5 + a6; a11 = a7 * a8; a12 = a9 - a10;
        a13 = a11 + a12; a14 = a13 * a1;
        
        CLOBBER_REGS;
        
        /* Conditional inside loop */
        if (i & 1) {
            a1 = a2 + a3; a4 = a5 * a6; a7 = a8 - a9;
        } else {
            a10 = a11 + a12; a13 = a14 * a1; a2 = a3 - a4;
        }
    }
    
    /* Use all variables */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                       "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
                       "r"(a11), "r"(a12), "r"(a13), "r"(a14) : "memory");
}

/* Main function exists only to make the file compilable */
int main(void) {
    /* Call high-pressure functions to ensure they're not eliminated */
    high_pressure_function();
    another_high_pressure_function(42);
    return 0;
}
