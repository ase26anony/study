/* test-mcf-debug.c
 * Designed to trigger debug dumping of fixup graph edges with
 * NEW_ENTRY and NEW_EXIT nodes in GCC's IRA min-cost flow solver.
 * Compile with: gcc-debug -O2 -march=armv7-a -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking to define MCF_DEBUG
 */

/* Force use of priority-based IRA algorithm */
#define OPT_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority"), \
                               target("arch=armv7-a")))

/* Volatile asm to clobber many ARM registers and prevent optimizations */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "memory")

/* High-pressure function with complex control flow */
OPT_ATTR
void high_pressure_function(void) {
    /* Declare many integer variables to create register pressure */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18;
    
    /* Force all variables to be live initially */
    CLOBBER_REGS;
    
    /* Complex control flow with multiple basic blocks */
    /* Each branch uses different subsets of variables */
    
    /* Block 1: Use first subset */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    CLOBBER_REGS;
    
    /* Conditional branch creating control flow complexity */
    if (v1 > 0) {
        /* Block 2: Different subset */
        v10 = v11 + v12;
        v13 = v14 * v15;
        v16 = v17 - v18;
        v2 = v10 + v13;
        CLOBBER_REGS;
        
        /* Nested condition */
        if (v16 > 0) {
            /* Block 3: Mix variables */
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 + v14;
            CLOBBER_REGS;
        } else {
            /* Block 4: Another mix */
            v15 = v16 + v17;
            v18 = v1 * v2;
            v3 = v4 - v5;
            v6 = v7 + v8;
            CLOBBER_REGS;
        }
        
        /* Block 5: More computations */
        v9 = v10 * v11;
        v12 = v13 - v14;
        v15 = v16 + v17;
    } else {
        /* Block 6: Alternative path */
        v18 = v1 + v2;
        v3 = v4 * v5;
        v6 = v7 - v8;
        v9 = v10 + v11;
        CLOBBER_REGS;
        
        /* Another nested condition */
        if (v18 < 100) {
            /* Block 7 */
            v12 = v13 * v14;
            v15 = v16 - v17;
            v1 = v2 + v3;
            v4 = v5 * v6;
            CLOBBER_REGS;
        } else {
            /* Block 8 */
            v7 = v8 + v9;
            v10 = v11 * v12;
            v13 = v14 - v15;
            v16 = v17 + v18;
            CLOBBER_REGS;
        }
        
        /* Block 9 */
        v2 = v3 * v4;
        v5 = v6 - v7;
        v8 = v9 + v10;
    }
    
    /* Final block: Use all variables to ensure extended liveness */
    v11 = v12 + v13;
    v14 = v15 * v16;
    v17 = v18 - v1;
    v2 = v3 + v4;
    v5 = v6 * v7;
    v8 = v9 - v10;
    v11 = v12 + v13;
    v14 = v15 * v16;
    v17 = v18 - v1;
    
    /* Force all variables to escape */
    CLOBBER_REGS;
}

/* Additional high-pressure function with switch statement */
OPT_ATTR
void switch_pressure_function(int selector) {
    int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    int a6 = 6, a7 = 7, a8 = 8, a9 = 9, a10 = 10;
    int a11 = 11, a12 = 12, a13 = 13, a14 = 14;
    
    CLOBBER_REGS;
    
    /* Switch creates multiple basic blocks */
    switch (selector & 0x7) {
        case 0:
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
            break;
        case 1:
            a10 = a11 + a12;
            a13 = a14 * a1;
            a2 = a3 - a4;
            break;
        case 2:
            a5 = a6 + a7;
            a8 = a9 * a10;
            a11 = a12 - a13;
            break;
        case 3:
            a14 = a1 + a2;
            a3 = a4 * a5;
            a6 = a7 - a8;
            break;
        case 4:
            a9 = a10 + a11;
            a12 = a13 * a14;
            a1 = a2 - a3;
            break;
        case 5:
            a4 = a5 + a6;
            a7 = a8 * a9;
            a10 = a11 - a12;
            break;
        case 6:
            a13 = a14 + a1;
            a2 = a3 * a4;
            a5 = a6 - a7;
            break;
        default:
            a8 = a9 + a10;
            a11 = a12 * a13;
            a14 = a1 - a2;
            break;
    }
    
    /* Cross-block computations */
    a3 = a4 + a5;
    a6 = a7 * a8;
    a9 = a10 - a11;
    a12 = a13 + a14;
    
    CLOBBER_REGS;
}

/* Main exists only to make the file compilable */
int main(void) {
    high_pressure_function();
    switch_pressure_function(3);
    return 0;
}
