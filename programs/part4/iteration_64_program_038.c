/* test_mcf_coverage.c
 * Designed to trigger debug dumping of fixup graph edges with
 * NEW_ENTRY and NEW_EXIT nodes in GCC's IRA min-cost flow solver.
 * Must be compiled with a GCC built with --enable-checking.
 */

/* Force use of priority-based IRA algorithm */
#define IRA_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited registers */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Clobber many ARM registers to increase pressure */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14", "memory")

/* Main high-pressure function */
IRA_PRIORITY ARM_TARGET
void high_pressure_ira_test(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with complex dependencies */
    v1 = cond1 * 2;
    v2 = cond2 + 7;
    v3 = cond3 - 3;
    v4 = v1 * v2;
    v5 = v2 + v3;
    v6 = v4 - v5;
    v7 = v1 + v6;
    v8 = v3 * v7;
    v9 = v5 / (v2 != 0 ? v2 : 1);
    v10 = v8 ^ v9;
    v11 = v6 | v10;
    v12 = v4 & v11;
    v13 = v7 + v12;
    v14 = v9 - v13;
    v15 = v10 * v14;
    v16 = v11 + v15;
    
    CLOBBER_REGS; /* Force many registers to appear used */
    
    /* Complex control flow to create different live ranges */
    if (cond1 > 0) {
        /* Use all variables in block 1 */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        v10 = v11 ^ v12;
        v13 = v14 | v15;
        v16 = v1 + v4 + v7 + v10 + v13;
        CLOBBER_REGS;
        
        if (cond2 < 0) {
            /* Nested block with different variable usage */
            v2 = v3 * v4;
            v5 = v6 + v7;
            v8 = v9 - v10;
            v11 = v12 ^ v13;
            v14 = v15 | v16;
            v1 = v2 + v5 + v8 + v11 + v14;
            CLOBBER_REGS;
        } else {
            /* Alternative path */
            v3 = v4 * v5;
            v6 = v7 + v8;
            v9 = v10 - v11;
            v12 = v13 ^ v14;
            v15 = v16 | v1;
            v2 = v3 + v6 + v9 + v12 + v15;
            CLOBBER_REGS;
        }
    } else {
        /* Else branch with different variable combinations */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v16;
        v1 = v2 + v5 + v8 + v11 + v14;
        CLOBBER_REGS;
        
        switch (cond3 % 4) {
            case 0:
                v3 = v4 + v5 + v6;
                v7 = v8 * v9 * v10;
                break;
            case 1:
                v4 = v5 - v6 - v7;
                v8 = v9 / (v10 != 0 ? v10 : 1);
                break;
            case 2:
                v5 = v6 ^ v7 ^ v8;
                v9 = v10 | v11 | v12;
                break;
            default:
                v6 = v7 & v8 & v9;
                v10 = v11 + v12 + v13;
                break;
        }
        CLOBBER_REGS;
    }
    
    /* Final computation using all variables to keep them live */
    v1 = v1 + v2 + v3 + v4;
    v5 = v5 + v6 + v7 + v8;
    v9 = v9 + v10 + v11 + v12;
    v13 = v13 + v14 + v15 + v16;
    
    /* Force all results to be used */
    asm volatile("" : "+r"(v1), "+r"(v5), "+r"(v9), "+r"(v13));
}

/* Secondary function with loop to increase pressure */
IRA_PRIORITY ARM_TARGET
void loop_pressure_test(void) {
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    int b1 = 6, b2 = 7, b3 = 8, b4 = 9, b5 = 10;
    int c1 = 11, c2 = 12, c3 = 13, c4 = 14, c5 = 15;
    
    for (int i = 0; i < 100; i++) {
        /* Complex loop body with many live variables */
        a1 = a2 * a3 + i;
        a2 = a3 - a4 * i;
        a3 = a4 ^ a5;
        a4 = a5 | a1;
        a5 = a1 & a2;
        
        b1 = b2 + b3 - i;
        b2 = b3 * b4 / (i + 1);
        b3 = b4 ^ b5;
        b4 = b5 | b1;
        b5 = b1 & b2;
        
        c1 = c2 - c3 + i;
        c2 = c3 * c4 % (i + 2);
        c3 = c4 ^ c5;
        c4 = c5 | c1;
        c5 = c1 & c2;
        
        CLOBBER_REGS;
        
        /* Conditional inside loop */
        if (i % 3 == 0) {
            a1 = b1 + c1;
            a2 = b2 - c2;
        } else if (i % 3 == 1) {
            b1 = a1 * c1;
            b2 = a2 / (c2 != 0 ? c2 : 1);
        } else {
            c1 = a1 ^ b1;
            c2 = a2 | b2;
        }
    }
    
    /* Use all results */
    asm volatile("" : "+r"(a1), "+r"(a2), "+r"(a3), "+r"(a4), "+r"(a5),
                       "+r"(b1), "+r"(b2), "+r"(b3), "+r"(b4), "+r"(b5),
                       "+r"(c1), "+r"(c2), "+r"(c3), "+r"(c4), "+r"(c5));
}

/* Main exists only to make the file compilable */
int main(void) {
    high_pressure_ira_test(1, -1, 2);
    loop_pressure_test();
    return 0;
}
