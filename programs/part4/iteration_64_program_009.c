/* test_mcf_coverage.c
 * Designed to trigger debug output in GCC's min-cost flow solver
 * when compiled with a debug-enabled GCC (--enable-checking)
 */

/* Force use of priority-based IRA algorithm */
#define OPT_ATTR __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Target ARM for limited registers - adjust if testing on other arch */
#ifdef __arm__
#define TARGET_ATTR __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ATTR __attribute__((target("arch=x86-64")))
#endif

/* Volatile assembly to clobber registers and prevent optimizations */
#ifdef __arm__
#define CLOBBER_REGS asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r12")
#else
#define CLOBBER_REGS asm volatile("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15")
#endif

/* Main high-pressure function */
TARGET_ATTR OPT_ATTR
void high_pressure_function(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with complex expressions to ensure they're live */
    v1 = cond1 * 2;
    v2 = cond2 + 7;
    v3 = cond3 - 3;
    v4 = v1 * v2;
    v5 = v2 + v3;
    v6 = v4 - v5;
    v7 = v6 * 2;
    v8 = v7 + v1;
    v9 = v8 - v3;
    v10 = v9 * v4;
    v11 = v10 / 2;
    v12 = v11 + v5;
    v13 = v12 - v6;
    v14 = v13 * v7;
    v15 = v14 + v8;
    v16 = v15 - v9;
    
    CLOBBER_REGS; /* Force many registers to be considered clobbered */
    
    /* Complex control flow to create different live ranges */
    if (cond1 > 0) {
        /* Use subset 1 - keep v1-v8 live */
        v1 = v2 + v3;
        v4 = v1 * v5;
        v6 = v4 - v7;
        v8 = v6 * v2;
        CLOBBER_REGS;
        
        if (cond2 < 0) {
            /* Nested block - use v9-v16 */
            v9 = v10 + v11;
            v12 = v9 * v13;
            v14 = v12 - v15;
            v16 = v14 * v10;
            CLOBBER_REGS;
            
            /* Mix variables from both sets */
            v1 = v16 + v9;
            v8 = v1 - v12;
        } else {
            /* Alternative path - different mix */
            v3 = v14 + v15;
            v5 = v3 * v16;
            v7 = v5 - v9;
            v11 = v7 * v3;
            CLOBBER_REGS;
        }
        
        /* Force all variables live at block end */
        v2 = v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15;
    } else {
        /* Else branch - use different combinations */
        v10 = v11 * v12;
        v13 = v10 - v14;
        v15 = v13 * v16;
        v2 = v15 + v4;
        CLOBBER_REGS;
        
        switch (cond3 & 3) {
            case 0:
                v1 = v10 + v13;
                v6 = v1 * v15;
                break;
            case 1:
                v3 = v11 + v14;
                v8 = v3 * v16;
                break;
            case 2:
                v5 = v12 + v15;
                v9 = v5 * v2;
                break;
            default:
                v7 = v13 + v16;
                v4 = v7 * v10;
                break;
        }
        CLOBBER_REGS;
    }
    
    /* Final computation using all variables to ensure they're live through most of function */
    v1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    v9 = v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Force output to prevent dead code elimination */
    asm volatile("" : "+r"(v1), "+r"(v9) : : "memory");
}

/* Secondary function with loop to increase pressure */
TARGET_ATTR OPT_ATTR
void loop_pressure_function(int iterations) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex loop body with many live variables */
        a1 = i * 2;
        a2 = a1 + 1;
        a3 = a2 * a1;
        a4 = a3 - i;
        a5 = a4 + a2;
        a6 = a5 * a3;
        a7 = a6 - a4;
        a8 = a7 + a5;
        a9 = a8 * a6;
        a10 = a9 - a7;
        a11 = a10 + a8;
        a12 = a11 * a9;
        
        CLOBBER_REGS;
        
        /* Conditional inside loop */
        if (i & 1) {
            a1 = a12 + a10;
            a3 = a1 * a11;
            a5 = a3 - a9;
        } else {
            a2 = a11 + a9;
            a4 = a2 * a10;
            a6 = a4 - a8;
        }
        
        /* Force cross-iteration liveness through phi nodes */
        a7 = a1 + a2 + a3 + a4 + a5 + a6;
        a8 = a7 * i;
    }
    
    /* Use all variables at end */
    a9 = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12;
    asm volatile("" : "+r"(a9) : : "memory");
}

/* Simple main to make file compilable */
int main() {
    high_pressure_function(1, -1, 2);
    loop_pressure_function(10);
    return 0;
}
