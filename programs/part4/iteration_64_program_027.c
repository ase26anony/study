/* test_mcf_coverage.c
 * Designed to trigger debug dumping of min-cost flow fixup graphs
 * in GCC's IRA register allocator when compiled with MCF_DEBUG enabled.
 */

/* Force use of priority-based allocator which uses min-cost flow solver */
#define IRA_ALGO_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited register set (16 GP registers, some reserved) */
#define ARM_TARGET __attribute__((target("arch=armv7-a", "tune=cortex-a8")))

/* Clobber many ARM registers to increase pressure */
#define CLOBBER_MANY_ARM asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "memory")

/* Clobber many x86 registers for x86 testing */
#define CLOBBER_MANY_X86 asm volatile("" : : : \
    "eax", "ebx", "ecx", "edx", "esi", "edi", \
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory")

/* High pressure function with complex control flow */
ARM_TARGET IRA_ALGO_PRIORITY
void high_pressure_arm(void) {
    /* Declare many integer variables to create live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize all variables with different values */
    v0 = 1;  v1 = 2;  v2 = 3;  v3 = 4;  v4 = 5;  v5 = 6;
    v6 = 7;  v7 = 8;  v8 = 9;  v9 = 10; v10 = 11; v11 = 12;
    v12 = 13; v13 = 14; v14 = 15; v15 = 16;
    
    /* Complex control flow with many live variables across edges */
    /* Block 1: All variables live */
    CLOBBER_MANY_ARM;
    v0 = v1 + v2; v3 = v4 * v5; v6 = v7 - v8;
    v9 = v10 / 2; v11 = v12 | v13; v14 = v15 ^ v0;
    
    /* Branch creating divergent live ranges */
    if (v0 > 100) {
        /* Block 2: Subset 1 live */
        CLOBBER_MANY_ARM;
        v1 = v2 + v3; v4 = v5 * v6; v7 = v8 - v9;
        v10 = v11 / 3; v12 = v13 | v14; v15 = v0 ^ v1;
        
        if (v1 < 50) {
            /* Block 3: Different subset */
            CLOBBER_MANY_ARM;
            v2 = v3 + v4; v5 = v6 * v7; v8 = v9 - v10;
            v11 = v12 / 4; v13 = v14 | v15; v0 = v1 ^ v2;
        } else {
            /* Block 4: Another subset */
            CLOBBER_MANY_ARM;
            v3 = v4 + v5; v6 = v7 * v8; v9 = v10 - v11;
            v12 = v13 / 5; v14 = v15 | v0; v1 = v2 ^ v3;
        }
        
        /* Block 5: Merge point */
        CLOBBER_MANY_ARM;
        v4 = v5 + v6; v7 = v8 * v9; v10 = v11 - v12;
        v13 = v14 / 6; v15 = v0 | v1; v2 = v3 ^ v4;
    } else {
        /* Block 6: Alternative path */
        CLOBBER_MANY_ARM;
        v5 = v6 + v7; v8 = v9 * v10; v11 = v12 - v13;
        v14 = v15 / 7; v0 = v1 | v2; v3 = v4 ^ v5;
        
        /* Nested switch-like structure */
        switch (v5 % 4) {
            case 0:
                v6 = v7 + v8; v9 = v10 * v11; break;
            case 1:
                v7 = v8 + v9; v10 = v11 * v12; break;
            case 2:
                v8 = v9 + v10; v11 = v12 * v13; break;
            default:
                v9 = v10 + v11; v12 = v13 * v14; break;
        }
        
        /* Block 7: More computations */
        CLOBBER_MANY_ARM;
        v10 = v11 + v12; v13 = v14 * v15; v0 = v1 - v2;
        v3 = v4 / 8; v5 = v6 | v7; v8 = v9 ^ v10;
    }
    
    /* Final block: Use all variables again */
    CLOBBER_MANY_ARM;
    v11 = v12 + v13; v14 = v15 * v0; v1 = v2 - v3;
    v4 = v5 / 9; v6 = v7 | v8; v9 = v10 ^ v11;
    
    /* Force all variables to be used in final result */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3),
                     "r"(v4), "r"(v5), "r"(v6), "r"(v7),
                     "r"(v8), "r"(v9), "r"(v10), "r"(v11),
                     "r"(v12), "r"(v13), "r"(v14), "r"(v15));
}

/* Alternative x86 version for testing on x86 hosts */
__attribute__((optimize("O2", "-fira-algorithm=priority")))
void high_pressure_x86(void) {
    int v0 = 1, v1 = 2, v2 = 3, v3 = 4, v4 = 5, v5 = 6;
    int v6 = 7, v7 = 8, v8 = 9, v9 = 10, v10 = 11, v11 = 12;
    int v12 = 13, v13 = 14, v14 = 15, v15 = 16, v16 = 17, v17 = 18;
    
    /* Even more variables for x86's larger register file */
    CLOBBER_MANY_X86;
    
    /* Complex data flow graph */
    for (int i = 0; i < 10; i++) {
        /* Loop creates overlapping live ranges */
        v0 = v1 + v2 + i;
        v3 = v4 * v5 - i;
        v6 = v7 | v8 ^ i;
        v9 = v10 & v11 | i;
        v12 = v13 ^ v14 + i;
        v15 = v16 - v17 * i;
        
        /* Conditional inside loop */
        if (i & 1) {
            v1 = v2 + v3;
            v4 = v5 * v6;
            v7 = v8 - v9;
            v10 = v11 / (i + 1);
        } else {
            v2 = v3 + v4;
            v5 = v6 * v7;
            v8 = v9 - v10;
            v11 = v12 | (i + 2);
        }
        
        CLOBBER_MANY_X86;
    }
    
    /* Cross-connecting all variables */
    v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
         v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    
    asm volatile("" : : "r"(v0));
}

/* Function with irreducible control flow (increases graph complexity) */
ARM_TARGET IRA_ALGO_PRIORITY
void irreducible_cfg(void) {
    int a = 1, b = 2, c = 3, d = 4, e = 5, f = 6;
    int g = 7, h = 8, i = 9, j = 10, k = 11, l = 12;
    
    CLOBBER_MANY_ARM;
    
    /* Create irreducible region with multiple entry points */
    goto label2;
    
label1:
    a = b + c;
    d = e * f;
    if (a > d) goto label3;
    else goto label4;
    
label2:
    g = h + i;
    j = k * l;
    if (g < j) goto label1;
    else goto label3;
    
label3:
    b = c + d;
    e = f * g;
    if (b > e) goto label4;
    else goto label1;
    
label4:
    c = d + e;
    f = g * h;
    /* Use all variables */
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d),
                     "r"(e), "r"(f), "r"(g), "r"(h),
                     "r"(i), "r"(j), "r"(k), "r"(l));
}

/* Main function exists only to make file compilable */
int main(void) {
    /* The actual coverage happens at compile-time,
     * but we call functions to avoid dead code elimination */
    high_pressure_arm();
    high_pressure_x86();
    irreducible_cfg();
    return 0;
}
