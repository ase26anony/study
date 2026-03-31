/* test_mcf_coverage.c
 * Designed to trigger debug dumping of fixup graph edges in GCC's IRA
 * when compiled with a debug-enabled GCC (--enable-checking)
 */

/* Force ARM target for limited registers */
#ifdef __ARM_ARCH_7A__
#define TARGET_ATTR __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ATTR __attribute__((target("arch=x86-64")))
#endif

/* Force priority-based IRA algorithm */
#define IRA_OPTIMIZE __attribute__((optimize("O2", "-fira-algorithm=priority")))

/* Volatile assembly to clobber registers and prevent optimizations */
#ifdef __arm__
#define CLOBBER_REGS asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r12")
#else
#define CLOBBER_REGS asm volatile("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15")
#endif

/* High-pressure function with many simultaneously live variables */
TARGET_ATTR IRA_OPTIMIZE
static void high_pressure_function(void) {
    /* Declare 16 integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8;
    int v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with different values */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4;
    v5 = 5; v6 = 6; v7 = 7; v8 = 8;
    v9 = 9; v10 = 10; v11 = 11; v12 = 12;
    v13 = 13; v14 = 14; v15 = 15; v16 = 16;
    
    /* Complex computation making all variables live */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v10 = v11 / 2;
    v12 = v13 | v14;
    v15 = v16 & 0xFF;
    
    CLOBBER_REGS; /* Force many registers to be considered clobbered */
    
    /* Complex control flow to create different live ranges */
    int selector = v1;
    
    /* Switch with multiple cases to create different basic blocks */
    switch (selector & 0x7) {
        case 0:
            /* Use subset 1 */
            v2 = v3 + v4;
            v5 = v6 * v7;
            v8 = v9 - v10;
            v11 = v12 | v13;
            v14 = v15 & v16;
            v1 = v2 * v3;
            break;
            
        case 1:
            /* Use subset 2 */
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 | v14;
            v15 = v16 & v1;
            v2 = v3 * v4;
            break;
            
        case 2:
            /* Use subset 3 */
            v4 = v5 + v6;
            v7 = v8 * v9;
            v10 = v11 - v12;
            v13 = v14 | v15;
            v16 = v1 & v2;
            v3 = v4 * v5;
            break;
            
        case 3:
            /* Use subset 4 */
            v5 = v6 + v7;
            v8 = v9 * v10;
            v11 = v12 - v13;
            v14 = v15 | v16;
            v1 = v2 & v3;
            v4 = v5 * v6;
            break;
            
        case 4:
            /* Use subset 5 */
            v6 = v7 + v8;
            v9 = v10 * v11;
            v12 = v13 - v14;
            v15 = v16 | v1;
            v2 = v3 & v4;
            v5 = v6 * v7;
            break;
            
        case 5:
            /* Use subset 6 */
            v7 = v8 + v9;
            v10 = v11 * v12;
            v13 = v14 - v15;
            v16 = v1 | v2;
            v3 = v4 & v5;
            v6 = v7 * v8;
            break;
            
        case 6:
            /* Use subset 7 */
            v8 = v9 + v10;
            v11 = v12 * v13;
            v14 = v15 - v16;
            v1 = v2 | v3;
            v4 = v5 & v6;
            v7 = v8 * v9;
            break;
            
        default:
            /* Use all variables */
            v9 = v10 + v11;
            v12 = v13 * v14;
            v15 = v16 - v1;
            v2 = v3 | v4;
            v5 = v6 & v7;
            v8 = v9 * v10;
            v11 = v12 + v13;
            v14 = v15 * v16;
            break;
    }
    
    CLOBBER_REGS; /* Another clobber to split live ranges */
    
    /* More computations to keep variables live */
    v1 = v1 + v2;
    v3 = v3 + v4;
    v5 = v5 + v6;
    v7 = v7 + v8;
    v9 = v9 + v10;
    v11 = v11 + v12;
    v13 = v13 + v14;
    v15 = v15 + v16;
    
    /* Nested if-else chain for additional control flow complexity */
    if (v1 > 0) {
        v2 = v3 * v4;
        v5 = v6 + v7;
        if (v2 < 100) {
            v8 = v9 - v10;
            v11 = v12 | v13;
        } else {
            v14 = v15 & v16;
            v1 = v2 * v3;
        }
    } else if (v4 < 0) {
        v6 = v7 / 2;
        v8 = v9 * 3;
        v10 = v11 + v12;
    } else {
        v13 = v14 - v15;
        v16 = v1 | v2;
        v3 = v4 & v5;
    }
    
    /* Final use of all variables to ensure they're live until the end */
    volatile int result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
        v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    (void)result; /* Suppress unused warning */
}

/* Secondary function with different pressure pattern */
TARGET_ATTR IRA_OPTIMIZE
static void another_high_pressure_func(int param) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    a1 = param;
    a2 = a1 * 2;
    a3 = a2 + 1;
    a4 = a3 - param;
    a5 = a4 * 3;
    a6 = a5 / 2;
    a7 = a6 | 0xFF;
    a8 = a7 & 0x0F;
    a9 = a8 << 2;
    a10 = a9 >> 1;
    a11 = a10 ^ 0x55;
    a12 = a11 + a1;
    a13 = a12 - a2;
    a14 = a13 * a3;
    
    CLOBBER_REGS;
    
    /* Loop to create more pressure */
    for (int i = 0; i < 4; i++) {
        a1 = a2 + a3;
        a4 = a5 - a6;
        a7 = a8 * a9;
        a10 = a11 / (i + 1);
        a12 = a13 | a14;
        a2 = a3 & a4;
        
        if (i & 1) {
            a5 = a6 + a7;
            a8 = a9 * a10;
        } else {
            a11 = a12 - a13;
            a14 = a1 | a2;
        }
    }
    
    volatile int out = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + 
                      a9 + a10 + a11 + a12 + a13 + a14;
    (void)out;
}

/* Main function exists only to make the file compilable */
int main(void) {
    high_pressure_function();
    another_high_pressure_func(42);
    return 0;
}
