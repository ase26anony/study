/* test-mcf-debug.c
 * Designed to trigger debug dumping of IRA's min-cost flow fixup graph
 * with artificial NEW_ENTRY/NEW_EXIT nodes.
 * Compile with: gcc-debug -O2 -march=armv7-a -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking to define MCF_DEBUG
 */

/* Force use of priority-based IRA algorithm */
#define IRA_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited register set */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Volatile asm to clobber many registers and prevent optimizations */
#define CLOBBER_REGS asm volatile("" : : : "memory", \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14")

/* Function with extreme register pressure */
ARM_TARGET IRA_PRIORITY
void high_pressure_function(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize with complex expressions to create dependencies */
    v1 = cond1 * 2;
    v2 = cond2 + 7;
    v3 = cond3 - 3;
    v4 = v1 * v2;
    v5 = v2 + v3;
    v6 = v3 * v4;
    v7 = v4 - v5;
    v8 = v5 * v6;
    v9 = v6 + v7;
    v10 = v7 * v8;
    v11 = v8 - v9;
    v12 = v9 * v10;
    v13 = v10 + v11;
    v14 = v11 * v12;
    v15 = v12 - v13;
    
    CLOBBER_REGS; /* Force many registers to appear clobbered */
    
    /* Complex control flow to create intersecting live ranges */
    if (cond1 > 0) {
        /* Use all variables in block 1 */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        v10 = v11 + v12;
        v13 = v14 * v15;
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 + v13;
        v14 = v15 * v1;
        
        CLOBBER_REGS;
        
        if (cond2 < 0) {
            /* Nested block with different variable usage */
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 + v14;
            v15 = v1 * v2;
            v4 = v5 + v6;
            v7 = v8 * v9;
            v10 = v11 - v12;
            v13 = v14 + v15;
            
            CLOBBER_REGS;
        } else {
            /* Alternative path */
            v3 = v4 - v5;
            v6 = v7 + v8;
            v9 = v10 * v11;
            v12 = v13 - v14;
            v15 = v1 + v2;
            v4 = v5 - v6;
            v7 = v8 + v9;
            v10 = v11 * v12;
            v13 = v14 - v15;
            
            CLOBBER_REGS;
        }
        
        /* Merge point - use all variables again */
        v1 = v3 + v6;
        v2 = v4 * v7;
        v5 = v8 - v9;
        v10 = v11 + v12;
        v13 = v14 * v15;
        v3 = v1 + v2;
        v6 = v4 * v5;
        v9 = v7 - v8;
        v12 = v10 + v11;
        v15 = v13 * v14;
        
        CLOBBER_REGS;
    } else {
        /* Else branch with different variable usage pattern */
        v1 = v15 - v14;
        v2 = v13 * v12;
        v3 = v11 + v10;
        v4 = v9 - v8;
        v5 = v7 * v6;
        v6 = v5 + v4;
        v7 = v3 * v2;
        v8 = v1 - v15;
        v9 = v14 * v13;
        v10 = v12 + v11;
        
        CLOBBER_REGS;
        
        switch (cond3 & 3) {
            case 0:
                v11 = v10 * v9;
                v12 = v8 - v7;
                v13 = v6 + v5;
                v14 = v4 * v3;
                v15 = v2 - v1;
                break;
            case 1:
                v11 = v10 + v9;
                v12 = v8 * v7;
                v13 = v6 - v5;
                v14 = v4 + v3;
                v15 = v2 * v1;
                break;
            case 2:
                v11 = v10 - v9;
                v12 = v8 + v7;
                v13 = v6 * v5;
                v14 = v4 - v3;
                v15 = v2 + v1;
                break;
            default:
                v11 = v10 * v9;
                v12 = v8 + v7;
                v13 = v6 - v5;
                v14 = v4 * v3;
                v15 = v2 + v1;
                break;
        }
        
        CLOBBER_REGS;
    }
    
    /* Final computation using all variables to ensure they're live until end */
    v1 = v1 + v2 + v3;
    v4 = v4 * v5 * v6;
    v7 = v7 - v8 - v9;
    v10 = v10 + v11 + v12;
    v13 = v13 * v14 * v15;
    
    /* Force all variables to be observable */
    asm volatile("" : : "r"(v1), "r"(v4), "r"(v7), "r"(v10), "r"(v13));
}

/* Secondary function with different pressure pattern */
ARM_TARGET IRA_PRIORITY
void another_high_pressure_function(int iter) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize with loop-dependent values */
    a1 = iter;
    a2 = iter * 2;
    a3 = iter + 3;
    a4 = a1 * a2;
    a5 = a2 + a3;
    a6 = a3 * a4;
    a7 = a4 - a5;
    a8 = a5 * a6;
    a9 = a6 + a7;
    a10 = a7 * a8;
    a11 = a8 - a9;
    a12 = a9 * a10;
    a13 = a10 + a11;
    a14 = a11 * a12;
    
    CLOBBER_REGS;
    
    /* Loop with high pressure */
    for (int i = 0; i < 3; i++) {
        /* Rotate and update all variables */
        int t = a1;
        a1 = a2 + a3;
        a2 = a3 * a4;
        a3 = a4 - a5;
        a4 = a5 + a6;
        a5 = a6 * a7;
        a6 = a7 - a8;
        a7 = a8 + a9;
        a8 = a9 * a10;
        a9 = a10 - a11;
        a10 = a11 + a12;
        a11 = a12 * a13;
        a12 = a13 - a14;
        a13 = a14 + t;
        a14 = t * a1;
        
        CLOBBER_REGS;
        
        if (i & 1) {
            /* Additional pressure in alternate iterations */
            a1 = a1 ^ a2;
            a3 = a3 | a4;
            a5 = a5 & a6;
            a7 = a7 ^ a8;
            a9 = a9 | a10;
            a11 = a11 & a12;
            a13 = a13 ^ a14;
        }
    }
    
    /* Consume results */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                       "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10));
}

/* Main exists only to make the file compilable */
int main() {
    high_pressure_function(1, -1, 2);
    another_high_pressure_function(5);
    return 0;
}
