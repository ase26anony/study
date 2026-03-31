/* test-mcf-debug.c
 * Designed to trigger debug dumping of fixup graph with NEW_ENTRY/NEW_EXIT nodes
 * Compile with: gcc-debug -O2 -march=armv7-a -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking to define MCF_DEBUG
 */

/* Force priority-based IRA algorithm */
#define OPT_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority"), \
                               target("arch=armv7-a", "no-thumb")))

/* Volatile assembly to clobber ARM registers and increase pressure */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "memory")

/* Function with extreme register pressure */
OPT_ATTR
void high_pressure_func(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with complex expressions to prevent optimization */
    v1 = cond1 * 2 + 1;
    v2 = cond2 * 3 - 2;
    v3 = cond3 * 5 + 3;
    v4 = v1 + v2 * v3;
    v5 = v2 - v3 / (v1 | 1);
    v6 = v3 ^ v1 & v2;
    v7 = (v4 << 2) | (v5 >> 1);
    v8 = v6 * v7 - v4;
    v9 = v5 + v8 * 3;
    v10 = v9 - v7 / 2;
    v11 = v10 ^ v8 & v9;
    v12 = v11 * 7 + v10;
    v13 = v12 - v11 / 3;
    v14 = v13 | v12 & v11;
    v15 = v14 * 9 - v13;
    v16 = v15 + v14 / 4;
    
    CLOBBER_REGS; /* Force many registers to appear used */
    
    /* Complex control flow to create different live ranges across blocks */
    if (cond1 > 0) {
        /* Use all variables in block 1 */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v7 - v8;
        v9 = v10 * v11;
        v12 = v13 + v14;
        v15 = v16 - v1;
        CLOBBER_REGS;
        
        if (cond2 < 0) {
            /* Nested block with different variable usage */
            v2 = v3 * v4;
            v5 = v6 + v7;
            v8 = v9 - v10;
            v11 = v12 * v13;
            v14 = v15 + v16;
            v1 = v2 - v3;
            CLOBBER_REGS;
            
            /* Force another level */
            for (int i = 0; i < 3; i++) {
                v3 = v4 + v5 + i;
                v6 = v7 * v8 - i;
                v9 = v10 ^ v11;
                v12 = v13 | v14;
                v15 = v16 & v1;
                v2 = v3 * i;
            }
        } else {
            /* Alternative path */
            v3 = v4 - v5;
            v6 = v7 * v8;
            v9 = v10 + v11;
            v12 = v13 - v14;
            v15 = v16 * v1;
            v2 = v3 + v4;
            CLOBBER_REGS;
        }
        
        /* Merge point with all variables live */
        v4 = v5 + v6 + v7;
        v8 = v9 * v10 * v11;
        v12 = v13 - v14 - v15;
        v16 = v1 * v2 * v3;
    } else {
        /* Else branch with different usage pattern */
        v5 = v6 * v7;
        v8 = v9 + v10;
        v11 = v12 - v13;
        v14 = v15 * v16;
        v1 = v2 + v3;
        v4 = v5 - v6;
        CLOBBER_REGS;
        
        switch (cond3 & 3) {
            case 0:
                v7 = v8 * v9;
                v10 = v11 + v12;
                v13 = v14 - v15;
                v16 = v1 * v2;
                v3 = v4 + v5;
                v6 = v7 - v8;
                break;
            case 1:
                v8 = v9 + v10;
                v11 = v12 * v13;
                v14 = v15 - v16;
                v1 = v2 + v3;
                v4 = v5 * v6;
                v7 = v8 - v9;
                break;
            case 2:
                v9 = v10 * v11;
                v12 = v13 + v14;
                v15 = v16 - v1;
                v2 = v3 * v4;
                v5 = v6 + v7;
                v8 = v9 - v10;
                break;
            default:
                v10 = v11 + v12;
                v13 = v14 * v15;
                v16 = v1 - v2;
                v3 = v4 + v5;
                v6 = v7 * v8;
                v9 = v10 - v11;
                break;
        }
        
        CLOBBER_REGS;
    }
    
    /* Final use of all variables to keep them live until end */
    int result = 
        v1 + v2 - v3 * v4 + v5 / (v6 | 1) + 
        v7 ^ v8 & v9 | v10 + v11 - v12 * v13 + 
        v14 / (v15 | 1) + v16;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(result));
}

/* Secondary function with different pressure pattern */
OPT_ATTR
void another_pressure_func(int x) {
    int a1 = x * 2, a2 = x + 3, a3 = x - 4, a4 = x ^ 5;
    int a5 = a1 * a2, a6 = a3 + a4, a7 = a5 - a6, a8 = a7 ^ a1;
    int a9 = a2 * a3, a10 = a4 + a5, a11 = a6 - a7, a12 = a8 ^ a9;
    int a13 = a10 * a11, a14 = a12 + a13, a15 = a14 - a9, a16 = a15 ^ a10;
    
    CLOBBER_REGS;
    
    /* Loop with varying live ranges */
    for (int i = 0; i < 4; i++) {
        if (i & 1) {
            a1 = a2 + a3 + i;
            a4 = a5 * a6 - i;
            a7 = a8 ^ a9;
            a10 = a11 | a12;
            a13 = a14 & a15;
            a16 = a1 * i;
        } else {
            a2 = a3 - a4 + i;
            a5 = a6 * a7 - i;
            a8 = a9 ^ a10;
            a11 = a12 | a13;
            a14 = a15 & a16;
            a1 = a2 * i;
        }
        CLOBBER_REGS;
    }
    
    /* Force all to be used */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                     "r"(a5), "r"(a6), "r"(a7), "r"(a8),
                     "r"(a9), "r"(a10), "r"(a11), "r"(a12),
                     "r"(a13), "r"(a14), "r"(a15), "r"(a16));
}

/* Main just to make it compilable */
int main() {
    high_pressure_func(1, -1, 2);
    another_pressure_func(42);
    return 0;
}
