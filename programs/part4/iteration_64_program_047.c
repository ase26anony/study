/* test-mcf-coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * when compiled with a debug-enabled GCC (--enable-checking)
 */

/* Force use of priority-based IRA algorithm */
#define OPT_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited registers (16 GPRs, some reserved) */
#define ARM_ATTR __attribute__((target("arch=armv7-a")))

/* Function with extreme register pressure */
ARM_ATTR OPT_ATTR
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize with volatile reads to prevent constant propagation */
    asm volatile("" : "=r"(v1), "=r"(v2), "=r"(v3), "=r"(v4), 
                       "=r"(v5), "=r"(v6), "=r"(v7), "=r"(v8) : :);
    asm volatile("" : "=r"(v9), "=r"(v10), "=r"(v11), "=r"(v12),
                       "=r"(v13), "=r"(v14), "=r"(v15), "=r"(v16) : :);
    
    /* Complex control flow to create many live ranges across blocks */
    /* Block 1: All variables live */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v10 = v11 ^ v12;
    v13 = v14 | v15;
    
    /* Clobber many registers to increase pressure */
    asm volatile("" : : : "memory", 
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                 "r8", "r9", "r10", "r11", "r12");
    
    /* Conditional block 2: Different subsets live */
    if (v1 > 0) {
        /* Use first subset */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v16;
        
        /* More clobbering */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4");
        
        if (v2 < 100) {
            /* Nested condition keeps more vars live */
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 ^ v14;
            v15 = v16 | v1;
        } else {
            /* Alternative path */
            v4 = v5 + v6;
            v7 = v8 * v9;
            v10 = v11 - v12;
            v13 = v14 ^ v15;
            v16 = v1 | v2;
        }
        
        /* Force all variables to be used before block ends */
        v1 = v1 + v16;
        v2 = v2 + v15;
        v3 = v3 + v14;
    } else {
        /* Else branch: different variable usage pattern */
        v16 = v15 + v14;
        v13 = v12 * v11;
        v10 = v9 - v8;
        v7 = v6 ^ v5;
        v4 = v3 | v2;
        
        /* Switch statement for more control flow complexity */
        switch (v16 & 3) {
            case 0:
                v1 = v2 + v3;
                v4 = v5 + v6;
                break;
            case 1:
                v7 = v8 + v9;
                v10 = v11 + v12;
                break;
            case 2:
                v13 = v14 + v15;
                v16 = v1 + v2;
                break;
            default:
                v3 = v4 + v5;
                v6 = v7 + v8;
                break;
        }
        
        /* Another clobber */
        asm volatile("" : : : "r5", "r6", "r7", "r8", "r9", "r10");
    }
    
    /* Final block: use all variables to ensure they're live throughout */
    v1 = v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Volatile write to prevent dead code elimination */
    asm volatile("" : : "r"(v1) : "memory");
}

/* Additional function with loop to increase pressure further */
ARM_ATTR OPT_ATTR
void pressure_with_loop(void) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
    
    /* Initialize */
    asm volatile("" : "=r"(a1), "=r"(a2), "=r"(a3), "=r"(a4),
                       "=r"(a5), "=r"(a6), "=r"(a7), "=r"(a8) : :);
    asm volatile("" : "=r"(a9), "=r"(a10), "=r"(a11), "=r"(a12) : :);
    
    /* Loop with many live variables */
    for (int i = 0; i < 10; i++) {
        /* Rotate values to keep all variables live */
        int t = a1;
        a1 = a2 + a3;
        a2 = a3 + a4;
        a3 = a4 + a5;
        a4 = a5 + a6;
        a5 = a6 + a7;
        a6 = a7 + a8;
        a7 = a8 + a9;
        a8 = a9 + a10;
        a9 = a10 + a11;
        a10 = a11 + a12;
        a11 = a12 + t;
        a12 = t + i;
        
        /* Conditional inside loop */
        if (i & 1) {
            a1 = a2 * a3;
            a4 = a5 * a6;
        } else {
            a7 = a8 * a9;
            a10 = a11 * a12;
        }
        
        /* Clobber registers periodically */
        if (i % 3 == 0) {
            asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5");
        }
    }
    
    /* Final use */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4) : "memory");
}

/* Main exists only to make the file compilable */
int main(void) {
    return 0;
}
