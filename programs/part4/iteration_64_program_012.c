/* test-mcf-coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * when compiled with a GCC built with --enable-checking
 */

/* Force use of priority-based IRA algorithm */
#define IRA_ALGORITHM __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM to increase register pressure */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Function with extreme register pressure */
ARM_TARGET IRA_ALGORITHM
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize with volatile reads to prevent constant propagation */
    asm volatile("" : "=r"(v0), "=r"(v1), "=r"(v2), "=r"(v3) : : "memory");
    asm volatile("" : "=r"(v4), "=r"(v5), "=r"(v6), "=r"(v7) : : "memory");
    asm volatile("" : "=r"(v8), "=r"(v9), "=r"(v10), "=r"(v11) : : "memory");
    asm volatile("" : "=r"(v12), "=r"(v13), "=r"(v14), "=r"(v15) : : "memory");
    
    /* Complex control flow to create many basic blocks */
    /* First level of branching */
    if (v0 > 0) {
        /* Use all variables in this block to keep them live */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        v10 = v11 ^ v12;
        v13 = v14 | v15;
        
        /* Clobber many registers to increase pressure */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", 
                     "r7", "r8", "r9", "r10", "r12", "memory");
        
        /* Second level of branching */
        if (v1 < v4) {
            v2 = v3 + v7;
            v5 = v6 * v10;
            v8 = v9 - v13;
            v11 = v12 ^ v0;
            v14 = v15 | v4;
            
            /* More register clobbering */
            asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "memory");
        } else {
            v3 = v7 + v10;
            v6 = v13 * v0;
            v9 = v4 - v11;
            v12 = v8 ^ v14;
            v15 = v2 | v5;
            
            asm volatile("" : : : "r5", "r6", "r7", "r8", "r9", "memory");
        }
        
        /* Third level - switch statement for more blocks */
        switch (v2 & 3) {
            case 0:
                v0 = v1 + v2 + v3;
                v4 = v5 + v6 + v7;
                break;
            case 1:
                v8 = v9 + v10 + v11;
                v12 = v13 + v14 + v15;
                break;
            case 2:
                v0 = v4 + v8 + v12;
                v1 = v5 + v9 + v13;
                break;
            default:
                v2 = v6 + v10 + v14;
                v3 = v7 + v11 + v15;
                break;
        }
    } else {
        /* Alternative path with different variable usage */
        v15 = v14 + v13;
        v12 = v11 + v10;
        v9 = v8 + v7;
        v6 = v5 + v4;
        v3 = v2 + v1;
        
        /* Nested loop-like structure */
        for (int i = 0; i < 3; i++) {
            v0 = v15 - v12;
            v1 = v12 - v9;
            v2 = v9 - v6;
            v3 = v6 - v3;
            
            /* Force spill/reload behavior */
            asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", 
                         "r6", "r7", "r8", "memory");
        }
    }
    
    /* Final computation using all variables */
    v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
         v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    /* Volatile write to prevent dead code elimination */
    asm volatile("" : : "r"(v0) : "memory");
}

/* Another high-pressure function with different pattern */
ARM_TARGET IRA_ALGORITHM
void another_pressure_function(int param) {
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13;
    
    /* Initialize from parameter to create dependencies */
    a0 = param;
    a1 = a0 + 1;
    a2 = a1 * 2;
    a3 = a2 - a0;
    a4 = a3 | a1;
    a5 = a4 ^ a2;
    a6 = a5 & a3;
    a7 = a6 << 2;
    a8 = a7 >> 1;
    a9 = a8 + a0;
    a10 = a9 * a1;
    a11 = a10 - a2;
    a12 = a11 | a3;
    a13 = a12 ^ a4;
    
    /* Deeply nested conditionals */
    if (a0 > 0) {
        if (a1 > a2) {
            if (a3 > a4) {
                a5 = a6 + a7 + a8;
                a9 = a10 + a11 + a12;
            } else {
                a6 = a7 + a8 + a9;
                a10 = a11 + a12 + a13;
            }
        } else {
            if (a4 > a5) {
                a7 = a8 + a9 + a10;
                a11 = a12 + a13 + a0;
            } else {
                a8 = a9 + a10 + a11;
                a12 = a13 + a0 + a1;
            }
        }
        
        /* Parallel computations */
        int t0 = a0 + a2 + a4 + a6 + a8 + a10 + a12;
        int t1 = a1 + a3 + a5 + a7 + a9 + a11 + a13;
        int t2 = t0 * t1;
        int t3 = t2 / (param + 1);
        
        asm volatile("" : : "r"(t3) : "memory", "r0", "r1", "r2", "r3");
    }
    
    /* Ensure all variables are used */
    asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                   "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9),
                   "r"(a10), "r"(a11), "r"(a12), "r"(a13) : "memory");
}

/* Main function - just to make the file compilable */
int main(void) {
    /* Call pressure functions with different parameters */
    high_pressure_function();
    another_pressure_function(42);
    return 0;
}
