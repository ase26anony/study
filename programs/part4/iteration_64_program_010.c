/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * when compiled with a GCC built with --enable-checking (MCF_DEBUG defined)
 */

/* Force use of priority-based IRA algorithm */
#define IRA_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited registers (16 GP registers, some reserved) */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Volatile assembly to clobber many registers */
#define CLOBBER_REGS asm volatile("" : : : "memory", \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14")

/* Main high-pressure function */
IRA_PRIORITY ARM_TARGET
void high_pressure_function(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with complex expressions to prevent optimization */
    v1 = cond1 * 2 + 1;
    v2 = cond2 * 3 - 2;
    v3 = cond3 * 4 + 3;
    v4 = v1 + v2 * v3;
    v5 = v2 - v3 / (v1 + 1);
    v6 = v3 ^ v1 | v2;
    v7 = v4 * v5 - v6;
    v8 = v5 + v6 * v7;
    v9 = v6 - v7 / (v8 + 1);
    v10 = v7 ^ v8 | v9;
    v11 = v8 * v9 - v10;
    v12 = v9 + v10 * v11;
    v13 = v10 - v11 / (v12 + 1);
    v14 = v11 ^ v12 | v13;
    v15 = v12 * v13 - v14;
    v16 = v13 + v14 * v15;
    
    /* Clobber registers to increase perceived pressure */
    CLOBBER_REGS;
    
    /* Complex control flow to create many live ranges across blocks */
    if (cond1 > 0) {
        /* Use all variables in block 1 */
        v1 = v2 + v3 + v4;
        v5 = v6 * v7 - v8;
        v9 = v10 ^ v11 | v12;
        v13 = v14 + v15 * v16;
        v2 = v3 - v4 / (v5 + 1);
        v6 = v7 ^ v8 | v9;
        
        CLOBBER_REGS;
        
        if (cond2 < 0) {
            /* Block 1a - different variable usage */
            v3 = v4 + v5 + v6;
            v7 = v8 * v9 - v10;
            v11 = v12 ^ v13 | v14;
            v15 = v16 + v1 * v2;
            v4 = v5 - v6 / (v7 + 1);
            v8 = v9 ^ v10 | v11;
            
            CLOBBER_REGS;
            
            /* Nested condition */
            if (cond3 == 0) {
                v5 = v6 + v7 + v8;
                v9 = v10 * v11 - v12;
                v13 = v14 ^ v15 | v16;
                v1 = v2 + v3 * v4;
            } else {
                v6 = v7 + v8 + v9;
                v10 = v11 * v12 - v13;
                v14 = v15 ^ v16 | v1;
                v2 = v3 + v4 * v5;
            }
        } else {
            /* Block 1b */
            v4 = v5 + v6 + v7;
            v8 = v9 * v10 - v11;
            v12 = v13 ^ v14 | v15;
            v16 = v1 + v2 * v3;
            v5 = v6 - v7 / (v8 + 1);
            v9 = v10 ^ v11 | v12;
            
            CLOBBER_REGS;
        }
        
        /* Merge point - use all variables again */
        v1 = v1 + v2 - v3 * v4;
        v5 = v5 | v6 ^ v7 & v8;
        v9 = v9 + v10 * v11 - v12;
        v13 = v13 ^ v14 | v15 & v16;
    } else {
        /* Block 2 - alternative path with different variable usage */
        v2 = v3 + v4 + v5;
        v6 = v7 * v8 - v9;
        v10 = v11 ^ v12 | v13;
        v14 = v15 + v16 * v1;
        v3 = v4 - v5 / (v6 + 1);
        v7 = v8 ^ v9 | v10;
        
        CLOBBER_REGS;
        
        /* Switch-like structure for more control flow */
        switch (cond2 & 3) {
            case 0:
                v4 = v5 + v6 + v7;
                v8 = v9 * v10 - v11;
                v12 = v13 ^ v14 | v15;
                break;
            case 1:
                v5 = v6 + v7 + v8;
                v9 = v10 * v11 - v12;
                v13 = v14 ^ v15 | v16;
                break;
            case 2:
                v6 = v7 + v8 + v9;
                v10 = v11 * v12 - v13;
                v14 = v15 ^ v16 | v1;
                break;
            default:
                v7 = v8 + v9 + v10;
                v11 = v12 * v13 - v14;
                v15 = v16 ^ v1 | v2;
                break;
        }
        
        CLOBBER_REGS;
    }
    
    /* Final computation using all variables to ensure they're live at end */
    v1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    v9 = v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    v2 = v1 * v9 - v3 * v4 + v5 * v6 - v7 * v8;
    v10 = v11 ^ v12 | v13 & v14 ^ v15 | v16;
    
    /* Force all variables to be observable */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                       "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                       "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                       "r"(v13), "r"(v14), "r"(v15), "r"(v16));
}

/* Secondary function with different pressure pattern */
IRA_PRIORITY ARM_TARGET
void another_high_pressure_func(int iter) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    a1 = iter * 1; a2 = iter * 2; a3 = iter * 3; a4 = iter * 4;
    a5 = iter * 5; a6 = iter * 6; a7 = iter * 7; a8 = iter * 8;
    a9 = iter * 9; a10 = iter * 10; a11 = iter * 11; a12 = iter * 12;
    a13 = iter * 13; a14 = iter * 14;
    
    /* Loop to create more complex live ranges */
    for (int i = 0; i < 3; i++) {
        CLOBBER_REGS;
        
        /* Complex data flow within loop */
        a1 = a2 + a3 - a4 * a5;
        a6 = a7 ^ a8 | a9 & a10;
        a11 = a12 * a13 - a14 / (a1 + 1);
        a2 = a3 + a4 - a5 * a6;
        a7 = a8 ^ a9 | a10 & a11;
        a12 = a13 * a14 - a1 / (a2 + 1);
        a3 = a4 + a5 - a6 * a7;
        a8 = a9 ^ a10 | a11 & a12;
        a13 = a14 * a1 - a2 / (a3 + 1);
        
        if (i & 1) {
            a4 = a5 + a6 - a7 * a8;
            a9 = a10 ^ a11 | a12 & a13;
            a14 = a1 * a2 - a3 / (a4 + 1);
        }
    }
    
    /* Use all variables */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                       "r"(a5), "r"(a6), "r"(a7), "r"(a8),
                       "r"(a9), "r"(a10), "r"(a11), "r"(a12),
                       "r"(a13), "r"(a14));
}

/* Main exists only to make the file compilable */
int main() {
    high_pressure_function(1, -1, 0);
    another_high_pressure_func(2);
    return 0;
}
