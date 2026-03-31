/* test_mcf_coverage.c
 * Designed to trigger debug dumping of fixup graph edges with special nodes
 * Compile with: gcc-debug -O2 -march=armv7-a -c test_mcf_coverage.c -o test.o
 * Requires GCC built with --enable-checking to define MCF_DEBUG
 */

/* Force use of priority-based IRA algorithm */
#define IRA_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited registers (16 GPRs, some reserved) */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Volatile assembly to clobber many registers */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14", "memory")

/* Main high-pressure function */
ARM_TARGET IRA_PRIORITY
void high_pressure_ira_test(void) {
    /* Declare many integer variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize with volatile values to prevent constant propagation */
    CLOBBER_REGS;
    v0 = 1;  v1 = 2;  v2 = 3;  v3 = 4;
    v4 = 5;  v5 = 6;  v6 = 7;  v7 = 8;
    v8 = 9;  v9 = 10; v10 = 11; v11 = 12;
    v12 = 13; v13 = 14; v14 = 15; v15 = 16;
    
    /* Complex control flow with overlapping live ranges */
    /* Block 1: All variables live */
    v0 = v1 + v2;
    v3 = v4 * v5;
    v6 = v7 - v8;
    v9 = v10 / (v11 ? v11 : 1);
    v12 = v13 | v14;
    v15 = v0 ^ v3;
    
    CLOBBER_REGS;  /* Force spills/reloads */
    
    /* Conditional block creating divergent live ranges */
    if (v0 > v1) {
        /* Subset 1 live */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 | v13;
        v14 = v15 ^ v0;
        
        /* Make all variables live again */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        v10 = v11 | v12;
        v13 = v14 ^ v15;
    } else {
        /* Subset 2 live (different combination) */
        v1 = v2 * v3;
        v4 = v5 + v6;
        v7 = v8 | v9;
        v10 = v11 ^ v12;
        v13 = v14 - v15;
        
        /* Cross-block dependencies */
        v0 = v1 + v4;
        v3 = v7 * v10;
        v6 = v13 | v0;
        v9 = v3 ^ v6;
        v12 = v9 - v1;
    }
    
    CLOBBER_REGS;
    
    /* Second level of nesting for more complex CFG */
    switch (v0 & 3) {
        case 0:
            v1 = v2 + v3; v4 = v5 * v6; v7 = v8 - v9;
            v10 = v11 | v12; v13 = v14 ^ v15;
            break;
        case 1:
            v2 = v3 + v4; v5 = v6 * v7; v8 = v9 - v10;
            v11 = v12 | v13; v14 = v15 ^ v0;
            break;
        case 2:
            v3 = v4 + v5; v6 = v7 * v8; v9 = v10 - v11;
            v12 = v13 | v14; v15 = v0 ^ v1;
            break;
        default:
            v4 = v5 + v6; v7 = v8 * v9; v10 = v11 - v12;
            v13 = v14 | v15; v0 = v1 ^ v2;
    }
    
    /* Final computations keeping everything live */
    v1 = v0 + v15;
    v2 = v1 * v14;
    v3 = v2 - v13;
    v4 = v3 | v12;
    v5 = v4 ^ v11;
    v6 = v5 + v10;
    v7 = v6 * v9;
    v8 = v7 - v3;  /* Circular dependency */
    
    /* Force all results to be used */
    CLOBBER_REGS;
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3),
                     "r"(v4), "r"(v5), "r"(v6), "r"(v7),
                     "r"(v8), "r"(v9), "r"(v10), "r"(v11),
                     "r"(v12), "r"(v13), "r"(v14), "r"(v15));
}

/* Additional variant with different pressure pattern */
ARM_TARGET IRA_PRIORITY
void alternative_pressure_pattern(int seed) {
    int a0 = seed + 1, a1 = seed + 2, a2 = seed + 3, a3 = seed + 4;
    int a4 = seed + 5, a5 = seed + 6, a6 = seed + 7, a7 = seed + 8;
    int a8 = seed + 9, a9 = seed + 10, a10 = seed + 11, a11 = seed + 12;
    int a12 = seed + 13, a13 = seed + 14, a14 = seed + 15, a15 = seed + 16;
    
    /* Loop with carried dependencies to create web of interferences */
    for (int i = 0; i < 4; i++) {
        a0 = a1 + a2;
        a3 = a4 * a5;
        a6 = a7 - a8;
        a9 = a10 | a11;
        a12 = a13 ^ a14;
        a15 = a0 + a15;
        
        if (i & 1) {
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
            a10 = a11 | a12;
            a13 = a14 ^ a15;
        } else {
            a2 = a3 + a4;
            a5 = a6 * a7;
            a8 = a9 - a10;
            a11 = a12 | a13;
            a14 = a15 ^ a0;
        }
        
        CLOBBER_REGS;
    }
    
    /* Consume results */
    asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3),
                     "r"(a4), "r"(a5), "r"(a6), "r"(a7),
                     "r"(a8), "r"(a9), "r"(a10), "r"(a11),
                     "r"(a12), "r"(a13), "r"(a14), "r"(a15));
}

/* Simple main to make file compilable */
int main(void) {
    return 0;
}
