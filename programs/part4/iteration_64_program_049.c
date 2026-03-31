/* test_mcf_coverage.c
 * Designed to trigger debug dumping of min-cost flow fixup graph
 * with artificial source/sink nodes in GCC's IRA register allocator.
 * Must be compiled with a GCC built with --enable-checking.
 */

/* Force use of priority-based IRA algorithm */
#define OPT_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority"), \
                               target("arch=armv7-a")))

/* Volatile assembly to clobber many registers on ARM */
#define CLOBBER_REGS asm volatile("" : : : \
    "memory", \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r13", "r14")

/* Function with extreme register pressure */
OPT_ATTR
void high_pressure_func(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with different values to prevent optimization */
    v1 = cond1 + 1;
    v2 = cond1 * 2;
    v3 = cond2 + 3;
    v4 = cond2 * 4;
    v5 = cond3 + 5;
    v6 = cond3 * 6;
    v7 = v1 + v2;
    v8 = v3 + v4;
    v9 = v5 + v6;
    v10 = v7 * 2;
    v11 = v8 * 3;
    v12 = v9 * 4;
    v13 = v10 + v11;
    v14 = v11 + v12;
    v15 = v12 + v10;
    v16 = v13 + v14 + v15;
    
    /* Clobber registers to increase pressure */
    CLOBBER_REGS;
    
    /* Complex control flow with different variable usage patterns */
    if (cond1 > 0) {
        /* Use first subset of variables */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 / (v9 ? v9 : 1);
        CLOBBER_REGS;
        
        if (cond2 < 0) {
            /* Use different subset */
            v10 = v11 ^ v12;
            v13 = v10 | v14;
            v15 = v13 & v16;
            v2 = v15 << 2;
            CLOBBER_REGS;
        } else {
            /* Another subset */
            v3 = v16 - v15;
            v5 = v3 * v14;
            v7 = v5 + v13;
            v9 = v7 >> 1;
            CLOBBER_REGS;
        }
        
        /* Force all variables live here */
        v16 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    } else {
        /* Alternative path with different usage */
        v2 = v3 * v4;
        v5 = v2 + v6;
        v7 = v5 - v8;
        v9 = v7 * v10;
        CLOBBER_REGS;
        
        switch (cond3 & 3) {
            case 0:
                v11 = v12 + v13;
                v14 = v11 * v15;
                v16 = v14 ^ v1;
                break;
            case 1:
                v12 = v13 - v14;
                v15 = v12 & v16;
                v1 = v15 | v2;
                break;
            case 2:
                v13 = v14 ^ v15;
                v16 = v13 + v1;
                v2 = v16 * v3;
                break;
            default:
                v14 = v15 | v16;
                v1 = v14 - v2;
                v3 = v1 ^ v4;
                break;
        }
        CLOBBER_REGS;
        
        /* More computations to extend live ranges */
        v4 = v9 + v11;
        v6 = v4 * v12;
        v8 = v6 - v13;
        v10 = v8 / (v14 ? v14 : 1);
        v12 = v10 | v15;
        v14 = v12 & v16;
    }
    
    /* Final use of all variables to ensure they're live across most of function */
    v1 = v2 + v3;
    v4 = v1 - v5;
    v6 = v4 * v7;
    v8 = v6 ^ v9;
    v10 = v8 | v11;
    v12 = v10 & v13;
    v14 = v12 + v15;
    v16 = v14 - (v1 ? v1 : 1);
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                       "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                       "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                       "r"(v13), "r"(v14), "r"(v15), "r"(v16));
}

/* Secondary function with loop to increase pressure further */
OPT_ATTR
void pressure_with_loop(int iterations) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
    
    a1 = iterations;
    a2 = a1 * 2;
    a3 = a2 + 1;
    a4 = a3 - iterations;
    
    for (int i = 0; i < iterations; i++) {
        /* Rotate values to create complex live ranges */
        a5 = a1 + a2;
        a6 = a3 + a4;
        a7 = a5 * a6;
        a8 = a7 - a1;
        a9 = a8 ^ a2;
        a10 = a9 | a3;
        a11 = a10 & a4;
        a12 = a11 << 2;
        
        /* Update all variables for next iteration */
        a1 = a12 + i;
        a2 = a1 - a11;
        a3 = a2 * a10;
        a4 = a3 ^ a9;
        
        CLOBBER_REGS;
    }
    
    /* Force all variables live at exit */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                       "r"(a5), "r"(a6), "r"(a7), "r"(a8),
                       "r"(a9), "r"(a10), "r"(a11), "r"(a12));
}

/* Main exists only to make the file compilable */
int main() {
    high_pressure_func(1, -1, 2);
    pressure_with_loop(10);
    return 0;
}
