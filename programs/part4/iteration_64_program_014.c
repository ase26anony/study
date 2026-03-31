/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * when compiled with a debug-enabled GCC (--enable-checking)
 */

/* Force ARM target for limited registers */
#ifdef __ARM_ARCH_7A__
#define TARGET_ATTR __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ATTR __attribute__((target("arch=armv7-a")))
#endif

/* Function with priority-based IRA algorithm */
void TARGET_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority")))
high_pressure_flow()
{
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize with volatile values to prevent constant propagation */
    asm volatile("" : "=r"(v1), "=r"(v2), "=r"(v3), "=r"(v4) : : "memory");
    asm volatile("" : "=r"(v5), "=r"(v6), "=r"(v7), "=r"(v8) : : "memory");
    asm volatile("" : "=r"(v9), "=r"(v10), "=r"(v11), "=r"(v12) : : "memory");
    asm volatile("" : "=r"(v13), "=r"(v14), "=r"(v15), "=r"(v16) : : "memory");
    
    /* Complex control flow with many live ranges across blocks */
    if (v1 > 0) {
        /* Block A: Use many variables to keep them live */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v16;
        
        /* Clobber ARM registers to increase pressure */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                     "r8", "r9", "r10", "r12", "memory");
        
        /* More computations to maintain liveness */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        v10 = v11 ^ v12;
        v13 = v14 | v15;
        v16 = v1 + v2;
    } else {
        /* Block B: Different pattern but same variables live */
        v3 = v4 + v5;
        v6 = v7 * v8;
        v9 = v10 - v11;
        v12 = v13 ^ v14;
        v15 = v16 | v1;
        
        /* Clobber different registers */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "memory");
        
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v16;
    }
    
    /* Nested conditional to create more control flow edges */
    switch (v1 & 0x3) {
        case 0:
            v2 = v3 * v4;
            v5 = v6 + v7;
            break;
        case 1:
            v8 = v9 ^ v10;
            v11 = v12 | v13;
            break;
        case 2:
            v14 = v15 - v16;
            v1 = v2 * v3;
            break;
        default:
            v4 = v5 + v6;
            v7 = v8 ^ v9;
            break;
    }
    
    /* Final use of all variables to ensure they're live until the end */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                 "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                 "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                 "r"(v13), "r"(v14), "r"(v15), "r"(v16) : "memory");
}

/* Second function with different pressure pattern */
void TARGET_ATTR __attribute__((optimize("O3", "-fira-algorithm=priority")))
another_high_pressure()
{
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    asm volatile("" : "=r"(a1), "=r"(a2), "=r"(a3), "=r"(a4) : : "memory");
    asm volatile("" : "=r"(a5), "=r"(a6), "=r"(a7), "=r"(a8) : : "memory");
    asm volatile("" : "=r"(a9), "=r"(a10), "=r"(a11), "=r"(a12) : : "memory");
    asm volatile("" : "=r"(a13), "=r"(a14) : : "memory");
    
    /* Loop to create more complex live ranges */
    for (int i = 0; i < 10; i++) {
        if (i & 1) {
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
            a10 = a11 ^ a12;
            a13 = a14 | a1;
        } else {
            a2 = a3 + a4;
            a5 = a6 * a7;
            a8 = a9 - a10;
            a11 = a12 ^ a13;
            a14 = a1 | a2;
        }
        
        /* Clobber registers inside loop */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                     "r8", "r9", "r10", "memory");
    }
    
    /* Force all variables to be used at end */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
                 "r"(a5), "r"(a6), "r"(a7), "r"(a8),
                 "r"(a9), "r"(a10), "r"(a11), "r"(a12),
                 "r"(a13), "r"(a14) : "memory");
}

/* Main function exists only to make the file compilable */
int main()
{
    return 0;
}
