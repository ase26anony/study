/* Test case to trigger min-cost flow fixup graph debug dumps in GCC's IRA.
   Compile with a debug-built GCC (configured with --enable-checking) using:
     gcc-debug -O2 -fira-algorithm=priority -march=armv7-a -c test.c -o test.o
   The compilation process itself should trigger the uncovered lines in mcf.cc
   when the fixup graph is dumped during register allocation. */

/* Force the high-pressure function to use priority-based IRA */
#define IRA_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM to limit available registers */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Clobber many ARM registers to increase pressure */
#define CLOBBER_REGS asm volatile("" : : : "memory", \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14")

/* Main high-pressure function */
IRA_PRIORITY ARM_TARGET
void high_pressure_func(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create live ranges */
    int v1 = cond1 * 2;
    int v2 = cond2 + 7;
    int v3 = cond3 - 3;
    int v4 = v1 * v2;
    int v5 = v2 / (cond1 ? 2 : 1);
    int v6 = v3 + v4;
    int v7 = v4 - v5;
    int v8 = v5 * v6;
    int v9 = v6 + v7;
    int v10 = v7 * v8;
    int v11 = v8 - v9;
    int v12 = v9 + v10;
    int v13 = v10 * v11;
    int v14 = v11 - v12;
    int v15 = v12 + v13;
    int v16 = v13 * v14;
    
    /* Force all variables to be live across control flow */
    CLOBBER_REGS;
    
    /* Complex control flow to create different live ranges */
    if (cond1 > 0) {
        /* Use subset 1 */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        v10 = v11 + v12;
        v13 = v14 * v15;
        v16 = v1 + v4;
        CLOBBER_REGS;
        
        if (cond2 < 0) {
            /* Nested block with different subset */
            v2 = v3 * v4;
            v5 = v6 + v7;
            v8 = v9 - v10;
            v11 = v12 * v13;
            v14 = v15 + v16;
            CLOBBER_REGS;
        } else {
            /* Alternative path */
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 + v14;
            v15 = v16 * v1;
            CLOBBER_REGS;
        }
        
        /* Merge point - use all variables */
        v1 = v1 + v2 + v3;
        v4 = v4 * v5 * v6;
        v7 = v7 - v8 - v9;
        v10 = v10 + v11 + v12;
        v13 = v13 * v14 * v15;
        v16 = v16 + v1 + v4;
    } else {
        /* Else branch with different variable usage */
        v2 = v3 * v4 * v5;
        v6 = v7 + v8 + v9;
        v10 = v11 - v12 - v13;
        v14 = v15 * v16 * v1;
        CLOBBER_REGS;
        
        switch (cond3 % 4) {
            case 0:
                v1 = v2 + v3;
                v4 = v5 * v6;
                break;
            case 1:
                v7 = v8 - v9;
                v10 = v11 + v12;
                break;
            case 2:
                v13 = v14 * v15;
                v16 = v1 + v4;
                break;
            default:
                v2 = v3 * v4;
                v5 = v6 + v7;
                break;
        }
        CLOBBER_REGS;
    }
    
    /* Final computation using all variables to ensure they're live at end */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                 v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Volatile store to prevent dead code elimination */
    asm volatile("" : : "r"(result) : "memory");
}

/* Secondary function with different pressure pattern */
IRA_PRIORITY ARM_TARGET
void another_high_pressure_func(int iter) {
    int a1 = iter * 1;
    int a2 = iter * 2;
    int a3 = iter * 3;
    int a4 = iter * 4;
    int a5 = iter * 5;
    int a6 = iter * 6;
    int a7 = iter * 7;
    int a8 = iter * 8;
    int a9 = iter * 9;
    int a10 = iter * 10;
    int a11 = iter * 11;
    int a12 = iter * 12;
    int a13 = iter * 13;
    int a14 = iter * 14;
    
    /* Loop to increase register pressure through live ranges */
    for (int i = 0; i < iter; i++) {
        /* Rotate values to keep all variables live */
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
    }
    
    /* Force all to be used */
    asm volatile("" : : 
        "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
        "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
        "r"(a11), "r"(a12), "r"(a13), "r"(a14) : "memory");
}

/* Main exists only to make the file compilable */
int main() {
    high_pressure_func(1, -1, 2);
    another_high_pressure_func(3);
    return 0;
}
