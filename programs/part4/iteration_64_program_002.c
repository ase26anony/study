/* test-mcf-debug.c
 * Designed to trigger debug dumping of fixup graph with NEW_ENTRY/NEW_EXIT nodes
 * Compile with: gcc-debug -O2 -march=armv7-a -c test-mcf-debug.c -o test.o
 * Requires GCC built with --enable-checking to define MCF_DEBUG
 */

/* Force use of priority-based IRA algorithm */
#define OPT_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited register set */
#define ARM_TARGET __attribute__((target("arch=armv7-a")))

/* Volatile assembly to clobber many registers */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "memory")

/* High-pressure function with many simultaneously live variables */
ARM_TARGET OPT_PRIORITY
void high_pressure_function(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with complex expressions to prevent optimization */
    v1 = cond1 * 2 + 1;
    v2 = cond2 * 3 - 2;
    v3 = cond3 * 5 + 3;
    v4 = v1 + v2 * 2;
    v5 = v2 - v3 / 2;
    v6 = v3 + v1 * 3;
    v7 = v4 * v5 - 6;
    v8 = v5 + v6 * 2;
    v9 = v6 - v7 / 3;
    v10 = v7 + v8 * 4;
    v11 = v8 - v9 * 2;
    v12 = v9 + v10 / 2;
    v13 = v10 * v11 + 7;
    v14 = v11 - v12 * 3;
    v15 = v12 + v13 / 4;
    v16 = v13 * v14 - 8;
    
    /* Clobber registers to increase perceived pressure */
    CLOBBER_REGS;
    
    /* Complex control flow to create different live ranges */
    if (cond1 > 0) {
        /* Use all variables in block 1 */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 / v9;
        v10 = v8 + v11;
        v12 = v10 * v13;
        v14 = v12 - v15;
        v16 = v14 / v1;
        
        if (cond2 > 0) {
            /* Nested block with different variable usage */
            v2 = v3 + v4;
            v5 = v2 * v6;
            v7 = v5 - v8;
            v9 = v7 / v10;
            v11 = v9 + v12;
            v13 = v11 * v14;
            v15 = v13 - v16;
            v1 = v15 / v2;
            
            CLOBBER_REGS;
        } else {
            /* Alternative path */
            v3 = v4 + v5;
            v6 = v3 * v7;
            v8 = v6 - v9;
            v10 = v8 / v11;
            v12 = v10 + v13;
            v14 = v12 * v15;
            v16 = v14 - v1;
            v2 = v16 / v3;
            
            CLOBBER_REGS;
        }
        
        /* More computations to keep variables live */
        v4 = v5 + v6;
        v7 = v4 * v8;
        v9 = v7 - v10;
        v11 = v9 / v12;
    } else {
        /* Else branch with different variable interactions */
        v5 = v6 + v7;
        v8 = v5 * v9;
        v10 = v8 - v11;
        v12 = v10 / v13;
        v14 = v12 + v15;
        v16 = v14 * v1;
        v2 = v16 - v3;
        v4 = v2 / v5;
        
        if (cond3 > 0) {
            /* Another nested block */
            v6 = v7 + v8;
            v9 = v6 * v10;
            v11 = v9 - v12;
            v13 = v11 / v14;
            v15 = v13 + v16;
            v1 = v15 * v2;
            v3 = v1 - v4;
            v5 = v3 / v6;
            
            CLOBBER_REGS;
        }
        
        /* Keep variables live across the else branch */
        v7 = v8 + v9;
        v10 = v7 * v11;
        v12 = v10 - v13;
    }
    
    /* Final complex use of all variables to ensure they remain live */
    v1 = v1 + v2 - v3 * v4 / (v5 + 1);
    v6 = v6 - v7 + v8 * v9 / (v10 + 2);
    v11 = v11 + v12 - v13 * v14 / (v15 + 3);
    v16 = v16 - v1 + v2 * v3 / (v4 + 4);
    
    /* One more clobber to prevent optimization */
    CLOBBER_REGS;
    
    /* Use all variables in final output-like expression */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                       "r"(v5), "r"(v6), "r"(v7), "r"(v8),
                       "r"(v9), "r"(v10), "r"(v11), "r"(v12),
                       "r"(v13), "r"(v14), "r"(v15), "r"(v16));
}

/* Secondary function with switch statement for more control flow complexity */
ARM_TARGET OPT_PRIORITY
void switch_pressure_function(int selector) {
    int a1 = selector * 2;
    int a2 = selector + 3;
    int a3 = selector - 4;
    int a4 = selector * 5;
    int a5 = selector + 6;
    int a6 = selector - 7;
    int a7 = selector * 8;
    int a8 = selector + 9;
    int a9 = selector - 10;
    int a10 = selector * 11;
    int a11 = selector + 12;
    int a12 = selector - 13;
    int a13 = selector * 14;
    int a14 = selector + 15;
    
    CLOBBER_REGS;
    
    /* Switch creates multiple basic blocks with overlapping live ranges */
    switch (selector % 5) {
        case 0:
            a1 = a2 + a3;
            a4 = a1 * a5;
            a6 = a4 - a7;
            a8 = a6 / a9;
            a10 = a8 + a11;
            break;
        case 1:
            a2 = a3 + a4;
            a5 = a2 * a6;
            a7 = a5 - a8;
            a9 = a7 / a10;
            a11 = a9 + a12;
            break;
        case 2:
            a3 = a4 + a5;
            a6 = a3 * a7;
            a8 = a6 - a9;
            a10 = a8 / a11;
            a12 = a10 + a13;
            break;
        case 3:
            a4 = a5 + a6;
            a7 = a4 * a8;
            a9 = a7 - a10;
            a11 = a9 / a12;
            a13 = a11 + a14;
            break;
        default:
            a5 = a6 + a7;
            a8 = a5 * a9;
            a10 = a8 - a11;
            a12 = a10 / a13;
            a14 = a12 + a1;
            break;
    }
    
    /* Cross-case variable usage */
    a1 = a1 + a2 + a3 + a4 + a5;
    a6 = a6 + a7 + a8 + a9 + a10;
    a11 = a11 + a12 + a13 + a14;
    
    CLOBBER_REGS;
    
    /* Force all variables to be used */
    asm volatile("" : : "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
                       "r"(a5), "r"(a6), "r"(a7), "r"(a8),
                       "r"(a9), "r"(a10), "r"(a11), "r"(a12),
                       "r"(a13), "r"(a14));
}

/* Main function exists only to make the file compilable */
int main() {
    /* Call pressure functions with different conditions */
    high_pressure_function(1, 0, 1);
    switch_pressure_function(42);
    return 0;
}
