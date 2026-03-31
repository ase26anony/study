/* test-mcf-debug.c
 * Designed to trigger debug dumping of min-cost flow fixup graph
 * in GCC's IRA register allocator when compiled with MCF_DEBUG enabled.
 */

/* Force use of priority-based register allocation algorithm */
#define IRA_ALGORITHM __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM to increase register pressure */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Volatile assembly to clobber many registers */
#define CLOBBER_REGS asm volatile("" : : : "memory", \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14")

/* Main high-pressure function */
ARM_TARGET IRA_ALGORITHM
void high_pressure_function(void) {
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with different values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5; v6 = 6; v7 = 7; v8 = 8;
    v9 = 9; v10 = 10; v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15; v16 = 16;
    
    /* Complex computation making all variables live */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v10 = v11 / 2;
    v12 = v13 | v14;
    v15 = v16 & 0xFF;
    
    CLOBBER_REGS; /* Force many registers to appear clobbered */
    
    /* Complex control flow to create many live ranges across blocks */
    if (v1 > 0) {
        /* Use different subsets in each branch */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 | 0x1;
        CLOBBER_REGS;
        
        if (v2 < 100) {
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 | v14;
            CLOBBER_REGS;
        } else {
            v13 = v14 + v15;
            v16 = v1 * v2;
            v4 = v5 - v6;
            v7 = v8 | v9;
            CLOBBER_REGS;
        }
        
        /* More computations to keep variables live */
        v10 = v11 + v12;
        v13 = v14 * v15;
        v16 = v1 - v2;
    } else {
        /* Alternative path with different variable usage */
        v14 = v15 + v16;
        v1 = v2 * v3;
        v4 = v5 - v6;
        v7 = v8 | v9;
        CLOBBER_REGS;
        
        switch (v14 % 4) {
            case 0:
                v10 = v11 + v12;
                v13 = v14 * v15;
                break;
            case 1:
                v16 = v1 - v2;
                v3 = v4 | v5;
                break;
            case 2:
                v6 = v7 + v8;
                v9 = v10 * v11;
                break;
            default:
                v12 = v13 - v14;
                v15 = v16 | v1;
                break;
        }
        CLOBBER_REGS;
    }
    
    /* Final complex use of all variables */
    v1 = v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Force the result to be used (prevents dead code elimination) */
    asm volatile("" : "+r"(v1));
}

/* Secondary function with different pressure pattern */
ARM_TARGET IRA_ALGORITHM
void another_high_pressure_function(int param) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize from parameter to create dependencies */
    a1 = param; a2 = param + 1; a3 = param + 2; a4 = param + 3;
    a5 = param + 4; a6 = param + 5; a7 = param + 6; a8 = param + 7;
    a9 = param + 8; a10 = param + 9; a11 = param + 10; a12 = param + 11;
    a13 = param + 12; a14 = param + 13;
    
    /* Loop to increase pressure */
    for (int i = 0; i < 10; i++) {
        a1 = a2 + a3;
        a4 = a5 * a6;
        a7 = a8 - a9;
        a10 = a11 / 2;
        a12 = a13 | a14;
        
        /* Rotate values to create complex live ranges */
        int tmp = a1;
        a1 = a2; a2 = a3; a3 = a4; a4 = a5; a5 = a6;
        a6 = a7; a7 = a8; a8 = a9; a9 = a10; a10 = a11;
        a11 = a12; a12 = a13; a13 = a14; a14 = tmp;
        
        CLOBBER_REGS;
    }
    
    /* Use all variables in final computation */
    int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14;
    asm volatile("" : "+r"(result));
}

/* Main exists only to make the file compilable */
int main(void) {
    high_pressure_function();
    another_high_pressure_function(42);
    return 0;
}
