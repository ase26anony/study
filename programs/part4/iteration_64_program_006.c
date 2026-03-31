/* test-mcf-debug.c
 * Designed to trigger debug dumping of min-cost flow fixup graph
 * in GCC's IRA register allocator when compiled with MCF_DEBUG enabled.
 */

/* Force use of priority-based IRA algorithm */
#define OPT_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited register set */
#ifdef __arm__
#define TARGET_ATTR __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ATTR __attribute__((target("arch=x86")))
#endif

/* Volatile assembly to clobber registers and prevent optimizations */
#ifdef __arm__
#define CLOBBER_REGS asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r12")
#else
#define CLOBBER_REGS asm volatile("" : : : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi")
#endif

/* High register pressure function with complex control flow */
TARGET_ATTR OPT_ATTR
void high_pressure_function(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with different values to prevent CSE */
    v1 = cond1 * 1;
    v2 = cond1 * 2;
    v3 = cond1 * 3;
    v4 = cond1 * 4;
    v5 = cond1 * 5;
    v6 = cond1 * 6;
    v7 = cond1 * 7;
    v8 = cond1 * 8;
    v9 = cond1 * 9;
    v10 = cond1 * 10;
    v11 = cond1 * 11;
    v12 = cond1 * 12;
    v13 = cond1 * 13;
    v14 = cond1 * 14;
    v15 = cond1 * 15;
    v16 = cond1 * 16;
    
    /* Clobber registers to increase perceived pressure */
    CLOBBER_REGS;
    
    /* Complex control flow with different live ranges */
    if (cond1 > 0) {
        /* Block A: Use subset 1 */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 / v9;
        CLOBBER_REGS;
        
        if (cond2 > 0) {
            /* Block B: Use subset 2, keep previous live */
            v10 = v8 + v11;
            v12 = v10 * v13;
            v14 = v12 - v15;
            v16 = v14 + v1;  /* v1 from previous block still live */
            CLOBBER_REGS;
        } else {
            /* Block C: Different subset, still many live */
            v3 = v4 + v5;
            v6 = v3 * v7;
            v8 = v6 - v9;
            v10 = v8 + v11;
            v12 = v10 * v13;
            CLOBBER_REGS;
        }
        
        /* Block D: Merge point - use many variables */
        v1 = v2 + v3 + v4 + v5;
        v6 = v7 * v8 * v9 * v10;
        v11 = v12 - v13 - v14 - v15;
        v16 = v1 + v6 + v11;
        CLOBBER_REGS;
    } else {
        /* Block E: Alternative path with different live set */
        v2 = v3 * v4;
        v5 = v6 + v7;
        v8 = v9 - v10;
        v11 = v12 * v13;
        v14 = v15 + v16;
        v1 = v2 + v5 + v8 + v11 + v14;
        CLOBBER_REGS;
        
        /* Nested switch-like structure */
        switch (cond3 & 3) {
            case 0:
                v3 = v4 + v5;
                v6 = v7 * v8;
                break;
            case 1:
                v9 = v10 - v11;
                v12 = v13 * v14;
                break;
            case 2:
                v15 = v16 + v1;
                v2 = v3 * v4;
                break;
            default:
                v5 = v6 + v7 + v8;
                v9 = v10 * v11 * v12;
                break;
        }
        CLOBBER_REGS;
    }
    
    /* Final computation using all variables to ensure they're live */
    v1 = v1 + v2 + v3 + v4;
    v5 = v5 * v6 * v7 * v8;
    v9 = v9 - v10 - v11 - v12;
    v13 = v13 + v14 + v15 + v16;
    
    /* Force all results to be used */
    asm volatile("" : "+r"(v1), "+r"(v5), "+r"(v9), "+r"(v13));
}

/* Additional high-pressure function with loop to increase IRA complexity */
TARGET_ATTR OPT_ATTR
void high_pressure_loop(int iterations) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
    
    for (int i = 0; i < iterations; i++) {
        /* Different computations in each iteration to prevent loop optimizations */
        a1 = i * 1;
        a2 = i * 2;
        a3 = i * 3;
        a4 = i * 4;
        a5 = i * 5;
        a6 = i * 6;
        a7 = i * 7;
        a8 = i * 8;
        a9 = i * 9;
        a10 = i * 10;
        a11 = i * 11;
        a12 = i * 12;
        
        /* Complex computation keeping many values live */
        a1 = a2 + a3;
        a4 = a5 * a6;
        a7 = a8 - a9;
        a10 = a11 / a12;
        
        /* Conditional inside loop */
        if (i & 1) {
            a2 = a3 + a4;
            a5 = a6 * a7;
        } else {
            a8 = a9 - a10;
            a11 = a12 * a1;
        }
        
        CLOBBER_REGS;
    }
    
    /* Use all variables after loop */
    asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4),
                       "+r"(a5), "+r"(a6), "+r"(a7), "+r"(a8),
                       "+r"(a9), "+r"(a10), "+r"(a11), "+r"(a12));
}

/* Main function exists only to make the file compilable */
int main() {
    high_pressure_function(1, 2, 3);
    high_pressure_loop(10);
    return 0;
}
