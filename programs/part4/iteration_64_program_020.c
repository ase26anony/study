/* test_mcf_coverage.c
 * Designed to trigger debug dumping of fixup graph edges with
 * NEW_ENTRY and NEW_EXIT nodes in GCC's IRA min-cost flow solver.
 * Compile with: gcc-debug -O2 -c test_mcf_coverage.c -o test.o
 * Or for ARM: gcc-debug -O2 -march=armv7-a -c test_mcf_coverage.c -o test.o
 */

/* Force priority-based IRA algorithm */
#ifdef __GNUC__
#define IRA_PRIORITY __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define IRA_PRIORITY
#endif

/* Target ARM for limited registers */
#ifdef __GNUC__
#define ARM_TARGET __attribute__((target("arch=armv7-a")))
#else
#define ARM_TARGET
#endif

/* Combine attributes */
#define HIGH_PRESSURE_FUNC ARM_TARGET IRA_PRIORITY

/* Volatile assembly to clobber registers and prevent optimizations */
#define CLOBBER_REGS_ARM asm volatile("" : : : "memory", \
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
    "r8", "r9", "r10", "r11", "r12", "r14")

#define CLOBBER_REGS_X86 asm volatile("" : : : "memory", \
    "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp")

/* Use appropriate clobber for architecture */
#ifdef __arm__
#define CLOBBER_ALL CLOBBER_REGS_ARM
#else
#define CLOBBER_ALL CLOBBER_REGS_X86
#endif

/* High-pressure function with many simultaneously live variables */
HIGH_PRESSURE_FUNC
void high_pressure_function(int cond1, int cond2, int cond3) {
    /* Declare many integer variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with complex expressions to ensure they're live */
    v1 = cond1 * 2 + 1;
    v2 = cond2 * 3 - 2;
    v3 = cond3 * 4 + 3;
    v4 = v1 + v2 * 2;
    v5 = v2 - v3 / 2;
    v6 = v3 + v1 * 3;
    v7 = v4 * v5 - 6;
    v8 = v5 + v6 * 2;
    v9 = v6 - v7 / 3;
    v10 = v7 + v8 * 4;
    v11 = v8 - v9 / 2;
    v12 = v9 + v10 * 3;
    v13 = v10 - v11 / 4;
    v14 = v11 + v12 * 2;
    v15 = v12 - v13 / 3;
    v16 = v13 + v14 * 5;
    
    /* Clobber registers to increase perceived pressure */
    CLOBBER_ALL;
    
    /* Complex control flow to create many live ranges across blocks */
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
        
        if (cond2 > 10) {
            /* Nested block with different variable usage */
            v2 = v3 * v4;
            v5 = v2 + v6;
            v7 = v5 - v8;
            v9 = v7 * v10;
            v11 = v9 + v12;
            v13 = v11 - v14;
            v15 = v13 * v16;
            v1 = v15 / v2;
            
            CLOBBER_ALL;
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
            
            CLOBBER_ALL;
        }
        
        /* More computations to keep variables live */
        v4 = v5 + v6 + v7;
        v8 = v9 * v10 * v11;
        v12 = v13 - v14 - v15;
    } else {
        /* Else branch with different variable interactions */
        v5 = v6 * v7;
        v8 = v5 + v9;
        v10 = v8 - v11;
        v12 = v10 / v13;
        v14 = v12 * v15;
        v16 = v14 + v1;
        v2 = v16 - v3;
        v4 = v2 * v5;
        
        CLOBBER_ALL;
        
        if (cond3 < 0) {
            /* Another nested conditional */
            v6 = v7 + v8;
            v9 = v6 * v10;
            v11 = v9 - v12;
            v13 = v11 / v14;
            v15 = v13 + v16;
            v1 = v15 * v2;
            v3 = v1 - v4;
            v5 = v3 / v6;
            
            CLOBBER_ALL;
        }
    }
    
    /* Final complex use of all variables to ensure they remain live */
    v1 = v2 + v3 + v4 + v5;
    v6 = v7 * v8 * v9 * v10;
    v11 = v12 - v13 - v14 - v15;
    v16 = v1 + v6 + v11;
    
    /* Use the result to prevent dead code elimination */
    asm volatile("" : "+r"(v16));
}

/* Another high-pressure function with switch statement for more control flow */
HIGH_PRESSURE_FUNC
void high_pressure_switch(int selector) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    a1 = selector * 1;
    a2 = selector * 2;
    a3 = selector * 3;
    a4 = selector * 4;
    a5 = selector * 5;
    a6 = selector * 6;
    a7 = selector * 7;
    a8 = selector * 8;
    a9 = selector * 9;
    a10 = selector * 10;
    a11 = selector * 11;
    a12 = selector * 12;
    a13 = selector * 13;
    a14 = selector * 14;
    
    CLOBBER_ALL;
    
    /* Switch creates multiple basic blocks */
    switch (selector & 7) {
        case 0:
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
            a10 = a11 / a12;
            break;
        case 1:
            a2 = a3 + a4;
            a5 = a6 * a7;
            a8 = a9 - a10;
            a11 = a12 / a13;
            break;
        case 2:
            a3 = a4 + a5;
            a6 = a7 * a8;
            a9 = a10 - a11;
            a12 = a13 / a14;
            break;
        case 3:
            a4 = a5 + a6;
            a7 = a8 * a9;
            a10 = a11 - a12;
            a13 = a14 / a1;
            break;
        case 4:
            a5 = a6 + a7;
            a8 = a9 * a10;
            a11 = a12 - a13;
            a14 = a1 / a2;
            break;
        case 5:
            a6 = a7 + a8;
            a9 = a10 * a11;
            a12 = a13 - a14;
            a1 = a2 / a3;
            break;
        case 6:
            a7 = a8 + a9;
            a10 = a11 * a12;
            a13 = a14 - a1;
            a2 = a3 / a4;
            break;
        default:
            a8 = a9 + a10;
            a11 = a12 * a13;
            a14 = a1 - a2;
            a3 = a4 / a5;
            break;
    }
    
    CLOBBER_ALL;
    
    /* More computations mixing all variables */
    a1 = a1 + a2 + a3 + a4 + a5;
    a6 = a6 * a7 * a8 * a9 * a10;
    a11 = a11 - a12 - a13 - a14;
    
    /* Force use of result */
    asm volatile("" : "+r"(a1), "+r"(a6), "+r"(a11));
}

/* Main function just to make the file compilable */
int main() {
    /* Call high-pressure functions with different conditions */
    high_pressure_function(1, 20, -5);
    high_pressure_switch(3);
    return 0;
}
