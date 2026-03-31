/* test_mcf_coverage.c
 * Compile with: gcc-debug -O2 -c test_mcf_coverage.c -o test.o
 * For ARM targeting: gcc-debug -O2 -march=armv7-a -c test_mcf_coverage.c -o test.o
 */

/* Force priority-based IRA algorithm */
#ifdef __GNUC__
#define FORCE_PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define FORCE_PRIORITY_IRA
#endif

/* Target ARM for limited registers */
#ifdef __GNUC__
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ARM
#endif

/* Function to create extreme register pressure */
FORCE_PRIORITY_IRA TARGET_ARM
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with different values to prevent optimization */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5; v6 = 6; v7 = 7; v8 = 8;
    v9 = 9; v10 = 10; v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15; v16 = 16;
    
    /* Clobber many registers to increase perceived pressure */
    asm volatile("" : : : "memory", 
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                 "r8", "r9", "r10", "r11", "r12", "r14");
    
    /* Complex control flow with many live variables across blocks */
    if (v1 > 0) {
        /* Block A: Use subset 1 */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 / (v13 ? v13 : 1);
        v14 = v15 ^ v16;
        
        /* Keep all variables live */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                     "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                     "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                     "r"(v13), "r"(v14), "r"(v15), "r"(v16) : "memory");
    } else {
        /* Block B: Use subset 2 (different combinations) */
        v3 = v1 + v2;
        v6 = v4 * v5;
        v9 = v7 - v8;
        v12 = v10 / (v11 ? v11 : 1);
        v15 = v13 ^ v14;
        v16 = v3 + v6;
        
        /* Keep all variables live */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                     "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                     "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                     "r"(v13), "r"(v14), "r"(v15), "r"(v16) : "memory");
    }
    
    /* Nested conditional to create more complex CFG */
    for (int i = 0; i < 3; i++) {
        switch (i) {
            case 0:
                v1 = v2 + v3;
                v4 = v5 * v6;
                break;
            case 1:
                v7 = v8 - v9;
                v10 = v11 ^ v12;
                break;
            case 2:
                v13 = v14 + v15;
                v16 = v1 * v2;
                break;
        }
        
        /* Force all variables to be live across loop iterations */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                     "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                     "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                     "r"(v13), "r"(v14), "r"(v15), "r"(v16) : "memory");
    }
    
    /* Final computation using all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                 v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
}

/* Second function with different pressure pattern */
FORCE_PRIORITY_IRA TARGET_ARM
void another_high_pressure_function(void) {
    /* Different set of variables */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    a1 = 100; a2 = 200; a3 = 300; a4 = 400; a5 = 500; a6 = 600; a7 = 700;
    a8 = 800; a9 = 900; a10 = 1000; a11 = 1100; a12 = 1200; a13 = 1300; a14 = 1400;
    
    /* Complex expression chain */
    a1 = a2 + a3;
    a4 = a5 * a6;
    a7 = a8 - a9;
    a10 = a11 / (a12 ? a12 : 1);
    a13 = a14 ^ a1;
    
    /* Conditional with phi nodes */
    int temp = a1 > a2 ? a3 + a4 : a5 - a6;
    
    /* Loop with carried dependencies */
    for (int i = 0; i < 4; i++) {
        a1 = a1 + temp;
        a2 = a2 * (i + 1);
        a3 = a3 - a4;
        a5 = a5 ^ a6;
        
        /* Clobber between iterations */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4");
    }
    
    /* Use all variables */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6),
                     "r"(a7), "r"(a8), "r"(a9), "r"(a10), "r"(a11), "r"(a12),
                     "r"(a13), "r"(a14) : "memory");
}

/* Main function exists only to make the file compilable */
int main(void) {
    /* The actual execution doesn't matter - coverage happens at compile time */
    return 0;
}
