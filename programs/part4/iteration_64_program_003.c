/* test-mcf-coverage.c
 * Designed to trigger debug dumping of fixup graph with NEW_ENTRY/NEW_EXIT nodes
 * Compile with: gcc-debug -O2 -march=armv7-a -c test-mcf-coverage.c -o test.o
 * Requires GCC built with --enable-checking to define MCF_DEBUG
 */

/* Force use of priority-based register allocator which uses min-cost flow solver */
#define FORCE_PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM to limit available registers */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Volatile assembly to clobber many registers and prevent optimizations */
#define CLOBBER_REGS asm volatile("" : : : "memory", \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14")

/* Main high-pressure function */
ARM_TARGET FORCE_PRIORITY_IRA
void high_pressure_function(void) {
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with different values to prevent coalescing */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5; v6 = 6; v7 = 7; v8 = 8;
    v9 = 9; v10 = 10; v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15; v16 = 16;
    
    CLOBBER_REGS; /* Force compiler to assume registers are modified */
    
    /* Complex control flow with many live ranges across blocks */
    /* Block 1: Use all variables in computations */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v10 = v11 / 2;
    v12 = v13 ^ v14;
    v15 = v16 << 1;
    
    /* Force all variables to be live across conditional */
    int condition = v1 + v4 + v7 + v10 + v12 + v15;
    CLOBBER_REGS;
    
    /* Multi-way branch creating complex control flow graph */
    switch (condition & 0x7) {  /* 8-way switch */
        case 0:
            /* Use different subset */
            v2 = v3 + v4; v5 = v6 * v7; v8 = v9 - v10;
            v11 = v12 / 2; v13 = v14 ^ v15; v16 = v1 << 1;
            break;
        case 1:
            /* Another subset */
            v3 = v4 + v5; v6 = v7 * v8; v9 = v10 - v11;
            v12 = v13 / 2; v14 = v15 ^ v16; v1 = v2 << 1;
            break;
        case 2:
            v4 = v5 + v6; v7 = v8 * v9; v10 = v11 - v12;
            v13 = v14 / 2; v15 = v16 ^ v1; v2 = v3 << 1;
            break;
        case 3:
            v5 = v6 + v7; v8 = v9 * v10; v11 = v12 - v13;
            v14 = v15 / 2; v16 = v1 ^ v2; v3 = v4 << 1;
            break;
        case 4:
            v6 = v7 + v8; v9 = v10 * v11; v12 = v13 - v14;
            v15 = v16 / 2; v1 = v2 ^ v3; v4 = v5 << 1;
            break;
        case 5:
            v7 = v8 + v9; v10 = v11 * v12; v13 = v14 - v15;
            v16 = v1 / 2; v2 = v3 ^ v4; v5 = v6 << 1;
            break;
        case 6:
            v8 = v9 + v10; v11 = v12 * v13; v14 = v15 - v16;
            v1 = v2 / 2; v3 = v4 ^ v5; v6 = v7 << 1;
            break;
        case 7:
            v9 = v10 + v11; v12 = v13 * v14; v15 = v16 - v1;
            v2 = v3 / 2; v4 = v5 ^ v6; v7 = v8 << 1;
            break;
    }
    
    CLOBBER_REGS;
    
    /* Final computations ensuring all variables are used */
    int result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
        v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
}

/* Additional function with loop to increase pressure */
ARM_TARGET FORCE_PRIORITY_IRA
void loop_pressure_function(void) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
    
    /* Initialize */
    a1 = 1; a2 = 2; a3 = 3; a4 = 4; a5 = 5; a6 = 6;
    a7 = 7; a8 = 8; a9 = 9; a10 = 10; a11 = 11; a12 = 12;
    
    /* Loop with many live variables */
    for (int i = 0; i < 100; i++) {
        /* Complex computations keeping all variables live */
        a1 = a2 + a3;
        a4 = a5 * a6;
        a7 = a8 - a9;
        a10 = a11 / (a12 + 1);
        
        /* Rotate values to create cross-iteration dependencies */
        int tmp = a1;
        a1 = a2; a2 = a3; a3 = a4; a4 = a5; a5 = a6;
        a6 = a7; a7 = a8; a8 = a9; a9 = a10; a10 = a11;
        a11 = a12; a12 = tmp;
        
        CLOBBER_REGS;
    }
    
    /* Use all variables */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12;
    asm volatile("" : : "r"(sum) : "memory");
}

/* Main exists only to make the file compilable */
int main(void) {
    high_pressure_function();
    loop_pressure_function();
    return 0;
}
