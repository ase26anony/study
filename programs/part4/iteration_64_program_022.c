/* test-mcf-debug.c
 * Designed to trigger debug dumping in GCC's min-cost flow solver
 * when compiled with a GCC built with --enable-checking (MCF_DEBUG defined)
 */

/* Force the compiler to use the priority-based allocator for this function */
#ifdef __GNUC__
#define FORCE_PRIORITY_ALLOC __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define FORCE_PRIORITY_ALLOC
#endif

/* Target ARM to limit available registers */
#ifdef __GNUC__
#define ARM_TARGET __attribute__((target("arch=armv7-a")))
#else
#define ARM_TARGET
#endif

/* Clobber many registers to increase pressure */
#define CLOBBER_REGS asm volatile("" : : : \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14", "memory")

/* For x86 testing alternative */
#define CLOBBER_X86 asm volatile("" : : : \
    "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", \
    "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "memory")

/* Main high-pressure function */
ARM_TARGET FORCE_PRIORITY_ALLOC
void high_pressure_function(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize with complex expressions to prevent optimization */
    v1 = cond1 * 2 + 1;
    v2 = cond2 * 3 - 1;
    v3 = cond3 * 5 + 2;
    v4 = v1 + v2 * 2;
    v5 = v2 - v3 * 3;
    v6 = v3 + v1 * 4;
    v7 = v4 * v5 - v6;
    v8 = v5 * v6 + v7;
    v9 = v6 * v7 - v8;
    v10 = v7 * v8 + v9;
    v11 = v8 * v9 - v10;
    v12 = v9 * v10 + v11;
    v13 = v10 * v11 - v12;
    v14 = v11 * v12 + v13;
    v15 = v12 * v13 - v14;
    v16 = v13 * v14 + v15;
    
    /* Complex control flow to create many live ranges across blocks */
    if (cond1 > 0) {
        /* Use all variables in block 1 */
        CLOBBER_REGS;
        v1 = v2 + v3 + v4;
        v5 = v6 * v7 - v8;
        v9 = v10 + v11 * v12;
        v13 = v14 - v15 * v16;
        v2 = v1 * v5 + v9;
        v3 = v13 - v16 / 2;
        
        if (cond2 < 0) {
            /* Nested block with different variable usage */
            CLOBBER_REGS;
            v4 = v5 + v6 + v7;
            v8 = v9 * v10 - v11;
            v12 = v13 + v14 * v15;
            v16 = v1 - v2 * v3;
            v7 = v4 * v8 + v12;
            v10 = v16 - v3 / 3;
            
            /* Force another level */
            if (cond3 == 0) {
                CLOBBER_REGS;
                v1 = v8 + v9 + v10;
                v2 = v11 * v12 - v13;
                v3 = v14 + v15 * v16;
                v4 = v5 - v6 * v7;
                v11 = v1 * v2 + v3;
                v14 = v4 - v7 / 4;
            }
        } else {
            /* Alternative path */
            CLOBBER_REGS;
            v5 = v8 + v9 + v10;
            v6 = v11 * v12 - v13;
            v7 = v14 + v15 * v16;
            v8 = v1 - v2 * v3;
            v12 = v5 * v6 + v7;
            v15 = v8 - v3 / 5;
        }
    } else {
        /* Else branch with different variable usage pattern */
        CLOBBER_REGS;
        v9 = v10 + v11 + v12;
        v10 = v13 * v14 - v15;
        v11 = v16 + v1 * v2;
        v12 = v3 - v4 * v5;
        v13 = v9 * v10 + v11;
        v16 = v12 - v5 / 6;
        
        /* Switch-like structure for more complexity */
        switch (cond2 & 3) {
            case 0:
                CLOBBER_REGS;
                v1 = v13 + v14 + v15;
                v2 = v16 * v9 - v10;
                break;
            case 1:
                CLOBBER_REGS;
                v3 = v11 + v12 + v13;
                v4 = v14 * v15 - v16;
                break;
            case 2:
                CLOBBER_REGS;
                v5 = v9 + v10 + v11;
                v6 = v12 * v13 - v14;
                break;
            default:
                CLOBBER_REGS;
                v7 = v15 + v16 + v1;
                v8 = v2 * v3 - v4;
                break;
        }
    }
    
    /* Final computation using all variables to ensure they're live */
    CLOBBER_REGS;
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + 
                 v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16;
    
    /* Use result to prevent dead code elimination */
    asm volatile("" : "+r" (result));
}

/* Second function with loop to increase pressure further */
ARM_TARGET FORCE_PRIORITY_ALLOC
void high_pressure_loop(int iterations) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16;
    
    /* Initialize */
    a1 = 1; a2 = 2; a3 = 3; a4 = 4;
    a5 = 5; a6 = 6; a7 = 7; a8 = 8;
    a9 = 9; a10 = 10; a11 = 11; a12 = 12;
    a13 = 13; a14 = 14; a15 = 15; a16 = 16;
    
    for (int i = 0; i < iterations; i++) {
        CLOBBER_REGS;
        /* Rotate and mix values to keep all variables live */
        int t = a1;
        a1 = a2 + a3 * i;
        a2 = a3 + a4 / (i + 1);
        a3 = a4 - a5 * i;
        a4 = a5 + a6 % (i + 2);
        a5 = a6 - a7 * i;
        a6 = a7 + a8 / (i + 3);
        a7 = a8 - a9 * i;
        a8 = a9 + a10 % (i + 4);
        a9 = a10 - a11 * i;
        a10 = a11 + a12 / (i + 5);
        a11 = a12 - a13 * i;
        a12 = a13 + a14 % (i + 6);
        a13 = a14 - a15 * i;
        a14 = a15 + a16 / (i + 7);
        a15 = a16 - t * i;
        a16 = t + a1 % (i + 8);
        
        /* Conditional inside loop */
        if (i % 3 == 0) {
            CLOBBER_REGS;
            a1 = a2 * a3 - a4;
            a5 = a6 + a7 * a8;
        } else if (i % 3 == 1) {
            CLOBBER_REGS;
            a9 = a10 * a11 - a12;
            a13 = a14 + a15 * a16;
        }
    }
    
    /* Force all variables to be used */
    int sum = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + 
              a9 + a10 + a11 + a12 + a13 + a14 + a15 + a16;
    asm volatile("" : "+r" (sum));
}

/* Main exists only to make the file compilable */
int main() {
    high_pressure_function(1, -1, 0);
    high_pressure_loop(10);
    return 0;
}
