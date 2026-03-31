/* test_mcf_coverage.c
 * Designed to trigger min-cost flow fixup graph debugging output
 * when compiled with GCC built with --enable-checking (MCF_DEBUG defined)
 */

/* Force use of priority-based IRA algorithm */
#ifdef __GNUC__
#define PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define PRIORITY_IRA
#endif

/* Target ARM for limited registers */
#ifdef __GNUC__
#define ARM_TARGET __attribute__((target("arch=armv7-a")))
#else
#define ARM_TARGET
#endif

/* Combined attributes for high-pressure function */
#define HIGH_PRESSURE_FUNC ARM_TARGET PRIORITY_IRA

/* Volatile assembly to clobber ARM registers and prevent optimizations */
#define CLOBBER_REGS asm volatile("" : : : "memory", \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14")

/* Create complex control flow with many live variables */
static HIGH_PRESSURE_FUNC void high_pressure_function(void)
{
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    int tmp;
    
    /* Initialize all variables with different values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5;
    v6 = 6; v7 = 7; v8 = 8; v9 = 9; v10 = 10;
    v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15;
    
    /* Force all variables to be live across control flow */
    CLOBBER_REGS;
    
    /* Complex conditional structure to create many basic blocks */
    /* Each branch uses different subsets of variables */
    
    if (v1 > 0) {
        /* Block A: Use variables 1-8 */
        tmp = v1 + v2 + v3;
        v4 = tmp * v5;
        v6 = v7 - v8;
        v4 = v4 + v6;
        CLOBBER_REGS;
    } else {
        /* Block B: Use variables 9-15 */
        tmp = v9 * v10;
        v11 = tmp + v12;
        v13 = v14 - v15;
        v11 = v11 * v13;
        CLOBBER_REGS;
    }
    
    /* Another level of conditionals */
    if (v4 > 100) {
        /* Block C: Mix variables from both sets */
        v1 = v9 + v10;
        v2 = v11 * v12;
        v3 = v13 - v14;
        v5 = v1 + v2 + v3;
        CLOBBER_REGS;
        
        /* Nested if for more control flow */
        if (v5 < 50) {
            /* Block D */
            v6 = v7 * v8;
            v9 = v10 + v11;
            v12 = v13 - v14;
            v15 = v6 + v9 + v12;
            CLOBBER_REGS;
        } else {
            /* Block E */
            v1 = v2 * v3;
            v4 = v5 + v6;
            v7 = v8 - v9;
            v10 = v1 + v4 + v7;
            CLOBBER_REGS;
        }
    } else {
        /* Block F: Different mix */
        v8 = v9 * v10;
        v11 = v12 + v13;
        v14 = v15 - v1;
        v2 = v8 + v11 + v14;
        CLOBBER_REGS;
        
        /* Switch-like structure */
        switch (v2 & 3) {
            case 0:
                v3 = v4 + v5;
                v6 = v7 * v8;
                break;
            case 1:
                v9 = v10 - v11;
                v12 = v13 * v14;
                break;
            case 2:
                v15 = v1 * v2;
                v3 = v4 + v5;
                break;
            default:
                v6 = v7 - v8;
                v9 = v10 * v11;
                break;
        }
        CLOBBER_REGS;
    }
    
    /* Final computation using all variables to ensure they're live until end */
    v1 = v1 + v2 + v3 + v4 + v5;
    v6 = v6 + v7 + v8 + v9 + v10;
    v11 = v11 + v12 + v13 + v14 + v15;
    
    /* Use the result to prevent dead code elimination */
    asm volatile("" : : "r"(v1), "r"(v6), "r"(v11));
}

/* Second high-pressure function with different pattern */
static HIGH_PRESSURE_FUNC void another_pressure_function(void)
{
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Different initialization pattern */
    a1 = 100; a2 = 200; a3 = 300; a4 = 400; a5 = 500;
    a6 = 600; a7 = 700; a8 = 800; a9 = 900; a10 = 1000;
    a11 = 1100; a12 = 1200; a13 = 1300; a14 = 1400;
    
    CLOBBER_REGS;
    
    /* Loop with high register pressure */
    for (int i = 0; i < 10; i++) {
        /* Different operations in each iteration */
        if (i & 1) {
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
        } else {
            a10 = a11 + a12;
            a13 = a14 * a1;
            a2 = a3 - a4;
        }
        
        /* Rotate values */
        int t = a1;
        a1 = a2; a2 = a3; a3 = a4; a4 = a5;
        a5 = a6; a6 = a7; a7 = a8; a8 = a9;
        a9 = a10; a10 = a11; a11 = a12; a12 = a13;
        a13 = a14; a14 = t;
        
        CLOBBER_REGS;
    }
    
    /* Force all to be live */
    asm volatile("" : : 
        "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
        "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
        "r"(a11), "r"(a12), "r"(a13), "r"(a14));
}

/* Main function exists only to make the file compilable */
int main(void)
{
    /* The functions are static, but we need to reference them
     * to prevent removal by the compiler */
    high_pressure_function();
    another_pressure_function();
    return 0;
}
