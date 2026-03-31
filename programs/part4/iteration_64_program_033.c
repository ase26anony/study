/* test-mcf-debug.c
 * Designed to trigger debug dumping of min-cost flow fixup graph
 * when compiled with GCC built with --enable-checking (MCF_DEBUG defined).
 * Coverage target: dump_fixup_edge special node labels in mcf.cc
 */

/* Force ARM target for limited registers */
#ifdef __ARM_ARCH_7A__
#define TARGET_ATTR __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ATTR __attribute__((target("arch=armv7-a")))
#endif

/* Use priority-based IRA algorithm */
#define IRA_OPTIMIZE __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Volatile asm to clobber ARM registers */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "memory")

/* Main high-pressure function */
TARGET_ATTR IRA_OPTIMIZE
void high_pressure_function(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create register pressure */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize all variables with complex expressions to prevent optimization */
    v0 = cond1 * 2;
    v1 = cond2 + 7;
    v2 = cond3 ^ 0xABCD;
    v3 = v0 * v1 - v2;
    v4 = v1 | v2;
    v5 = v2 & v0;
    v6 = v3 + v4;
    v7 = v4 - v5;
    v8 = v5 * v6;
    v9 = v6 / (v7 ? v7 : 1);
    v10 = v7 << 2;
    v11 = v8 >> 1;
    v12 = v9 ^ v10;
    v13 = v10 | v11;
    v14 = v11 & v12;
    v15 = v12 * v13 + v14;
    
    /* Complex control flow to create many live ranges across blocks */
    if (cond1 > 0) {
        /* Use all variables in block 1 */
        v0 = v1 + v2;
        v3 = v4 - v5;
        v6 = v7 * v8;
        v9 = v10 ^ v11;
        v12 = v13 | v14;
        v15 = v0 + v3 + v6;
        CLOBBER_REGS; /* Force register clobbering */
        
        if (cond2 < 0) {
            /* Nested block with different variable usage */
            v1 = v2 * v3;
            v4 = v5 + v6;
            v7 = v8 - v9;
            v10 = v11 ^ v12;
            v13 = v14 | v15;
            v0 = v1 + v4 + v7;
            CLOBBER_REGS;
        } else {
            /* Alternative path */
            v2 = v3 / (v4 ? v4 : 1);
            v5 = v6 << v7;
            v8 = v9 >> 1;
            v11 = v12 & v13;
            v14 = v15 | v0;
            v1 = v2 * v5 * v8;
            CLOBBER_REGS;
        }
        
        /* Merge point - use many variables */
        v3 = v0 + v1 + v2;
        v6 = v4 + v5 + v7;
        v9 = v8 + v10 + v11;
        v12 = v13 + v14 + v15;
    } else {
        /* Else branch with different variable usage pattern */
        v1 = v0 * 3;
        v4 = v3 / 2;
        v7 = v6 + 100;
        v10 = v9 - 50;
        v13 = v12 ^ 0xFF;
        v15 = v14 << 1;
        CLOBBER_REGS;
        
        switch (cond3 & 3) {
            case 0:
                v2 = v1 + v4;
                v5 = v7 * v10;
                v8 = v13 + v15;
                break;
            case 1:
                v2 = v1 - v4;
                v5 = v7 / (v10 ? v10 : 1);
                v8 = v13 ^ v15;
                break;
            case 2:
                v2 = v1 | v4;
                v5 = v7 & v10;
                v8 = v13 << 2;
                break;
            default:
                v2 = v1 ^ v4;
                v5 = v7 >> 1;
                v8 = v15 | 0x0F;
                break;
        }
        
        /* More computations to extend live ranges */
        v11 = v2 + v5 + v8;
        v12 = v1 * v4 * v7;
    }
    
    /* Final use of all variables to ensure they're live until the end */
    v0 = v1 + v2 + v3 + v4;
    v5 = v6 + v7 + v8 + v9;
    v10 = v11 + v12 + v13 + v14 + v15;
    
    /* Prevent dead code elimination */
    asm volatile("" : "+r"(v0), "+r"(v5), "+r"(v10));
}

/* Secondary function with different control flow pattern */
TARGET_ATTR IRA_OPTIMIZE
void another_high_pressure_func(int x) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    a1 = x;
    a2 = x * 2;
    a3 = x + 1;
    a4 = x ^ 0x1234;
    a5 = a1 + a2;
    a6 = a3 - a4;
    a7 = a5 * a6;
    a8 = a6 / (a7 ? a7 : 1);
    a9 = a7 << 3;
    a10 = a8 >> 2;
    a11 = a9 | a10;
    a12 = a10 & a11;
    a13 = a11 ^ a12;
    a14 = a12 + a13;
    
    /* Loop with high pressure */
    for (int i = 0; i < 3; i++) {
        if (i & 1) {
            a1 = a2 + a3;
            a4 = a5 - a6;
            a7 = a8 * a9;
            a10 = a11 ^ a12;
            a13 = a14 | a1;
        } else {
            a2 = a3 * a4;
            a5 = a6 + a7;
            a8 = a9 - a10;
            a11 = a12 & a13;
            a14 = a1 ^ a2;
        }
        CLOBBER_REGS;
    }
    
    /* Complex expression using all variables */
    int result = a1 + a2 - a3 * a4 + a5 / (a6 ? a6 : 1) + 
                 (a7 & a8) | (a9 ^ a10) + (a11 << 2) - (a12 >> 1) + 
                 a13 * a14;
    asm volatile("" : "+r"(result));
}

/* Main exists only to make the file compilable */
int main() {
    return 0;
}
