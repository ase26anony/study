/* test-mcf-debug.c
 * Designed to trigger debug dumping of fixup graph with NEW_ENTRY/NEW_EXIT nodes
 * Compile with: gcc-debug -O2 -c test-mcf-debug.c -o test.o
 * Or for ARM: gcc-debug -O2 -march=armv7-a -c test-mcf-debug.c -o test.o
 */

/* Force priority-based IRA algorithm */
#ifdef __GNUC__
#define FORCE_PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define FORCE_PRIORITY_IRA
#endif

/* Target ARM for fewer registers */
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
    
    /* Initialize with volatile asm to prevent constant propagation */
    asm volatile("" : "=r"(v1), "=r"(v2), "=r"(v3), "=r"(v4), 
                       "=r"(v5), "=r"(v6), "=r"(v7), "=r"(v8) : : "memory");
    asm volatile("" : "=r"(v9), "=r"(v10), "=r"(v11), "=r"(v12),
                       "=r"(v13), "=r"(v14), "=r"(v15), "=r"(v16) : : "memory");
    
    /* Complex control flow to create overlapping live ranges */
    /* First basic block: use all variables */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v10 = v11 / (v12 != 0 ? v12 : 1);
    v13 = v14 | v15;
    v16 = v1 ^ v4;
    
    /* Clobber many registers to increase pressure */
    asm volatile("" : : : 
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
        "r8", "r9", "r10", "r11", "r12", "r14", "memory");
    
    /* Branch point creating multiple paths */
    if (v1 > v2) {
        /* Path A: use subset 1 */
        v3 = v4 + v5;
        v6 = v7 * v8;
        v9 = v10 - v11;
        v12 = v13 | v14;
        /* Keep all variables live by using them */
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                           "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                           "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                           "r"(v13), "r"(v14), "r"(v15), "r"(v16) : "memory");
    } else if (v3 < v4) {
        /* Path B: use subset 2 */
        v5 = v6 + v7;
        v8 = v9 * v10;
        v11 = v12 - v13;
        v14 = v15 | v16;
        /* Different usage pattern */
        v1 = v2 ^ v3;
        v4 = v5 & v6;
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                           "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                           "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                           "r"(v13), "r"(v14), "r"(v15), "r"(v16) : "memory");
    } else {
        /* Path C: use subset 3 */
        v7 = v8 + v9;
        v10 = v11 * v12;
        v13 = v14 - v15;
        v16 = v1 | v2;
        v3 = v4 ^ v5;
        v6 = v7 & v8;
        v9 = v10 + v11;
        asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                           "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                           "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                           "r"(v13), "r"(v14), "r"(v15), "r"(v16) : "memory");
    }
    
    /* Another branch to create more control flow edges */
    switch (v1 & 0x3) {
        case 0:
            v2 = v3 + v4;
            v5 = v6 * v7;
            break;
        case 1:
            v8 = v9 + v10;
            v11 = v12 * v13;
            break;
        case 2:
            v14 = v15 + v16;
            v1 = v2 * v3;
            break;
        default:
            v4 = v5 + v6;
            v7 = v8 * v9;
            break;
    }
    
    /* Final computation using all variables to ensure they're live until end */
    v1 = v1 + v2 + v3 + v4;
    v5 = v5 + v6 + v7 + v8;
    v9 = v9 + v10 + v11 + v12;
    v13 = v13 + v14 + v15 + v16;
    
    /* Force all results to be observable */
    asm volatile("" : : "r"(v1), "r"(v5), "r"(v9), "r"(v13) : "memory");
}

/* Additional high-pressure function with different pattern */
FORCE_PRIORITY_IRA TARGET_ARM
void another_pressure_function(int param) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize from parameter to create dependencies */
    a1 = param;
    a2 = a1 + 1;
    a3 = a2 * 2;
    a4 = a3 - a1;
    a5 = a4 | a2;
    a6 = a5 ^ a3;
    a7 = a6 & a4;
    a8 = a7 + a5;
    a9 = a8 * a6;
    a10 = a9 - a7;
    a11 = a10 | a8;
    a12 = a11 ^ a9;
    a13 = a12 & a10;
    a14 = a13 + a11;
    
    /* Loop with high pressure */
    for (int i = 0; i < 4; i++) {
        /* Rotate values to keep all live */
        int t = a1;
        a1 = a2; a2 = a3; a3 = a4; a4 = a5;
        a5 = a6; a6 = a7; a7 = a8; a8 = a9;
        a9 = a10; a10 = a11; a11 = a12; a12 = a13;
        a13 = a14; a14 = t;
        
        /* Clobber registers */
        asm volatile("" : : : 
            "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
            "r8", "r9", "r10", "r11", "r12", "memory");
    }
    
    /* Complex conditional */
    if (a1 > 0) {
        a2 = a3 + a4;
        a5 = a6 * a7;
    } else if (a2 < 0) {
        a8 = a9 + a10;
        a11 = a12 * a13;
    } else {
        a14 = a1 + a2;
        a3 = a4 * a5;
    }
    
    /* Use all results */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                       "r"(a5), "r"(a6), "r"(a7), "r"(a8),
                       "r"(a9), "r"(a10), "r"(a11), "r"(a12),
                       "r"(a13), "r"(a14) : "memory");
}

/* Main function exists only to make the file compilable */
int main(void) {
    /* Call pressure functions with different parameters */
    high_pressure_function();
    another_pressure_function(42);
    return 0;
}
