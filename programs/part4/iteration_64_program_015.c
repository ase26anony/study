/* test_mcf_coverage.c
 * Designed to trigger debug dumping of fixup graph edges in GCC's IRA
 * when compiled with a debug-enabled GCC (--enable-checking).
 */

/* Force use of priority-based allocator for this function */
void __attribute__((optimize("O2", "-fira-algorithm=priority")))
__attribute__((target("arch=armv7-a")))
high_pressure_function(void)
{
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize with volatile reads to prevent constant propagation */
    asm volatile("" : "=r"(v1), "=r"(v2), "=r"(v3), "=r"(v4));
    
    /* Complex control flow with many live ranges */
    if (v1 > 0) {
        /* Block A: Use subset 1 of variables */
        v5 = v1 + v2;
        v6 = v3 * v4;
        v7 = v5 - v6;
        v8 = v2 + v7;
        
        /* Clobber many registers to increase pressure */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", 
                     "r5", "r6", "r7", "r8", "r9", "r10", "r12");
        
        /* More computations keeping variables live */
        v9 = v8 * v3;
        v10 = v4 + v9;
        v11 = v5 - v10;
    } else {
        /* Block B: Use subset 2 of variables */
        v12 = v2 * v3;
        v13 = v4 - v1;
        v14 = v12 + v13;
        v15 = v14 / 2;
        
        /* Different clobber set */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", 
                     "r5", "r6", "r7", "r8", "r9");
        
        v16 = v15 * v3;
        v7 = v16 + v4;
        v8 = v7 - v2;
    }
    
    /* Second level of control flow to create more complex CFG */
    switch (v8 & 3) {
        case 0:
            v9 = v1 + v2 + v3 + v4;
            v10 = v5 * v6 * v7;
            break;
        case 1:
            v11 = v2 + v3 + v4 + v5;
            v12 = v6 * v7 * v8;
            break;
        case 2:
            v13 = v3 + v4 + v5 + v6;
            v14 = v7 * v8 * v9;
            break;
        default:
            v15 = v4 + v5 + v6 + v7;
            v16 = v8 * v9 * v10;
            break;
    }
    
    /* Final computation using all variables to ensure extended liveness */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                 v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Force result to be used */
    asm volatile("" : : "r"(result));
}

/* Additional high-pressure function with different pattern */
void __attribute__((optimize("O3", "-fira-algorithm=priority")))
__attribute__((target("arch=armv7-a")))
another_pressure_function(int x)
{
    /* Even more variables */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18;
    
    /* Initialize from parameter and computations */
    a1 = x;
    a2 = x * 2;
    a3 = x + 1;
    a4 = x - 1;
    
    /* Loop with high pressure */
    for (int i = 0; i < 10; i++) {
        /* All variables live across loop iterations */
        a5 = a1 + a2;
        a6 = a3 * a4;
        a7 = a5 - a6 + i;
        a8 = a2 + a7;
        a9 = a8 * a3;
        a10 = a4 + a9;
        a11 = a5 - a10;
        a12 = a6 + a11;
        a13 = a7 * a12;
        a14 = a8 + a13;
        a15 = a9 - a14;
        a16 = a10 * a15;
        a17 = a11 + a16;
        a18 = a12 - a17;
        
        /* Heavy clobbering */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                     "r8", "r9", "r10", "r11", "r12", "r14");
    }
    
    /* Use all results */
    int total = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + 
                a11 + a12 + a13 + a14 + a15 + a16 + a17 + a18;
    asm volatile("" : : "r"(total));
}

/* Simple main to make file compilable */
int main(void)
{
    high_pressure_function();
    another_pressure_function(42);
    return 0;
}
