/* test_mcf_coverage.c
 * Designed to trigger debug dumping of fixup graph edges with
 * ENTRY/EXIT and NEW_ENTRY/NEW_EXIT nodes in GCC's IRA min-cost flow solver.
 * Must be compiled with a GCC built with --enable-checking (MCF_DEBUG defined).
 */

/* Force use of priority-based IRA algorithm which uses min-cost flow solver */
#define IRA_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARMv7-a for limited register set (16 GP registers, some reserved) */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Clobber many ARM registers to increase perceived pressure */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "memory")

/* Main high-pressure function */
ARM_TARGET IRA_PRIORITY
void high_pressure_func(void) {
    /* Declare many integer variables to create live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize with volatile values to prevent constant propagation */
    volatile int seed = 42;
    v0 = seed + 1;
    v1 = seed + 2;
    v2 = seed + 3;
    v3 = seed + 4;
    v4 = seed + 5;
    v5 = seed + 6;
    v6 = seed + 7;
    v7 = seed + 8;
    v8 = seed + 9;
    v9 = seed + 10;
    v10 = seed + 11;
    v11 = seed + 12;
    v12 = seed + 13;
    v13 = seed + 14;
    v14 = seed + 15;
    v15 = seed + 16;
    
    /* Complex control flow with many live variables across blocks */
    /* Block 1: All variables live */
    CLOBBER_REGS;
    v0 = v1 + v2;
    v3 = v4 * v5;
    v6 = v7 - v8;
    v9 = v10 ^ v11;
    v12 = v13 | v14;
    v15 = v0 & v3;
    
    /* Conditional block creating divergent live ranges */
    if (v0 > v1) {
        /* Block 2: Different subset live */
        CLOBBER_REGS;
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v0;
        v1 = v2 & v3;
        
        /* Nested condition */
        if (v2 < v3) {
            /* Block 3: Another subset */
            CLOBBER_REGS;
            v4 = v5 + v6;
            v7 = v8 * v9;
            v10 = v11 - v12;
            v13 = v14 ^ v15;
            v0 = v1 | v2;
            v3 = v4 & v5;
        } else {
            /* Block 4: Yet another subset */
            CLOBBER_REGS;
            v6 = v7 + v8;
            v9 = v10 * v11;
            v12 = v13 - v14;
            v15 = v0 ^ v1;
            v2 = v3 | v4;
            v5 = v6 & v7;
        }
        
        /* Block 5: Merge point - many variables become live again */
        CLOBBER_REGS;
        v8 = v9 + v10;
        v11 = v12 * v13;
        v14 = v15 - v0;
        v1 = v2 ^ v3;
        v4 = v5 | v6;
        v7 = v8 & v9;
    } else {
        /* Block 6: Alternative path */
        CLOBBER_REGS;
        v10 = v11 + v12;
        v13 = v14 * v15;
        v0 = v1 - v2;
        v3 = v4 ^ v5;
        v6 = v7 | v8;
        v9 = v10 & v11;
        
        /* Another nested condition */
        if (v10 > v11) {
            /* Block 7 */
            CLOBBER_REGS;
            v12 = v13 + v14;
            v15 = v0 * v1;
            v2 = v3 - v4;
            v5 = v6 ^ v7;
            v8 = v9 | v10;
            v11 = v12 & v13;
        }
        
        /* Block 8: Merge */
        CLOBBER_REGS;
        v14 = v15 + v0;
        v1 = v2 * v3;
        v4 = v5 - v6;
        v7 = v8 ^ v9;
        v10 = v11 | v12;
        v13 = v14 & v15;
    }
    
    /* Final block: Use all variables to ensure they're live until the end */
    CLOBBER_REGS;
    v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
         v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    /* Force use of result to prevent dead code elimination */
    volatile int *sink = (volatile int*)&seed;
    *sink = v0;
}

/* Second function with different pressure pattern to increase chances */
ARM_TARGET IRA_PRIORITY
void another_high_pressure_func(int cond) {
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13;
    
    volatile int init = 123;
    a0 = init; a1 = init+1; a2 = init+2; a3 = init+3;
    a4 = init+4; a5 = init+5; a6 = init+6; a7 = init+7;
    a8 = init+8; a9 = init+9; a10 = init+10; a11 = init+11;
    a12 = init+12; a13 = init+13;
    
    /* Switch statement creates multiple control flow edges */
    switch (cond & 7) {
        case 0:
            a0 = a1 * a2; a3 = a4 - a5; a6 = a7 ^ a8;
            CLOBBER_REGS;
            break;
        case 1:
            a1 = a2 * a3; a4 = a5 - a6; a7 = a8 ^ a9;
            CLOBBER_REGS;
            break;
        case 2:
            a2 = a3 * a4; a5 = a6 - a7; a8 = a9 ^ a10;
            CLOBBER_REGS;
            break;
        case 3:
            a3 = a4 * a5; a6 = a7 - a8; a9 = a10 ^ a11;
            CLOBBER_REGS;
            break;
        case 4:
            a4 = a5 * a6; a7 = a8 - a9; a10 = a11 ^ a12;
            CLOBBER_REGS;
            break;
        default:
            a5 = a6 * a7; a8 = a9 - a10; a11 = a12 ^ a13;
            CLOBBER_REGS;
            break;
    }
    
    /* Complex expression with many operands */
    int result = ((a0 * a1) + (a2 - a3)) | ((a4 ^ a5) & (a6 + a7)) -
                 ((a8 | a9) ^ (a10 & a11)) * (a12 - a13);
    
    volatile int *sink = (volatile int*)&init;
    *sink = result;
}

/* Simple main to make file compilable */
int main(void) {
    high_pressure_func();
    another_high_pressure_func(3);
    return 0;
}
