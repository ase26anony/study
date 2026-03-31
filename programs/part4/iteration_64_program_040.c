/* test_mcf_coverage.c
 * Designed to trigger debug dumping in GCC's min-cost flow solver
 * when compiled with a debug-enabled GCC (--enable-checking)
 */

/* Force use of priority-based register allocator for this function */
void __attribute__((optimize("O2", "-fira-algorithm=priority")))
__attribute__((target("arch=armv7-a")))
high_pressure_function(void)
{
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize with volatile reads to prevent constant propagation */
    asm volatile("" : "=r"(v1), "=r"(v2), "=r"(v3), "=r"(v4));
    
    /* Complex control flow to create many live ranges across blocks */
    if (v1 > 0) {
        /* Block A: Use subset 1 of variables */
        v5 = v1 + v2;
        v6 = v3 * v4;
        v7 = v5 - v6;
        v8 = v2 ^ v3;
        
        /* Clobber many registers to increase pressure */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", 
                     "r5", "r6", "r7", "r8", "r9", "r10", "r12");
        
        /* More computations keeping variables live */
        v9 = v7 | v8;
        v10 = v9 << 2;
    } else {
        /* Block B: Use subset 2 of variables */
        v11 = v2 + v4;
        v12 = v1 * v3;
        v13 = v11 ^ v12;
        v14 = v4 - v1;
        
        /* Different clobber pattern */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4");
        
        v15 = v13 & v14;
        v16 = v15 >> 1;
    }
    
    /* Merge point: make all variables live again */
    int control;
    asm volatile("mov %0, #0" : "=r"(control));
    
    switch (control) {
        case 0:
            v1 = v2 + v3 + v4 + v5;
            v6 = v7 - v8 - v9 - v10;
            break;
        case 1:
            v11 = v12 * v13 * v14;
            v15 = v16 / 2;
            break;
        case 2:
            v1 = v11 ^ v15;
            v2 = v12 | v16;
            break;
        default:
            v3 = v4 & v13;
            v5 = v6 ^ v14;
            break;
    }
    
    /* Final complex computation using all variables */
    int result = v1 + v2 - v3 * v4 / v5 | v6 & v7 ^ v8;
    result += v9 - v10 * v11 / v12 | v13 & v14 ^ v15 + v16;
    
    /* Force result to be used */
    asm volatile("" : : "r"(result));
}

/* Second function with different pressure pattern */
void __attribute__((optimize("O3", "-fira-algorithm=priority")))
__attribute__((target("arch=armv7-a")))
another_high_pressure_function(int param)
{
    /* Different set of variables */
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize from parameter to create dependencies */
    a1 = param;
    a2 = a1 + 1;
    a3 = a2 * 2;
    a4 = a3 - a1;
    
    /* Nested conditionals for complex CFG */
    for (int i = 0; i < 3; i++) {
        if (i == 0) {
            a5 = a1 + a2;
            a6 = a3 + a4;
            a7 = a5 * a6;
        } else if (i == 1) {
            a8 = a2 - a3;
            a9 = a4 - a1;
            a10 = a8 ^ a9;
        } else {
            a11 = a1 | a2;
            a12 = a3 & a4;
            a13 = a11 << a12;
        }
        
        /* Volatile asm to prevent optimization and clobber registers */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5",
                     "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14");
    }
    
    /* Use all variables in final computation */
    a14 = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13;
    asm volatile("" : : "r"(a14));
}

/* Simple main to make file compilable */
int main(void)
{
    high_pressure_function();
    another_high_pressure_function(42);
    return 0;
}
