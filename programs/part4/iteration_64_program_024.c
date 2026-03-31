/* test_mcf_fixup.c - Min-Cost Flow debug coverage test */
/* Compile with: gcc-debug -O2 -march=armv7-a -c test_mcf_fixup.c -o test.o */
/* Requires GCC built with --enable-checking to define MCF_DEBUG */

/* Force priority-based IRA algorithm for the high-pressure function */
#ifdef __GNUC__
#define FORCE_PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define FORCE_PRIORITY_IRA
#endif

/* Target ARM for limited register set */
#ifdef __GNUC__
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ARM
#endif

/* Function to create extreme register pressure */
TARGET_ARM FORCE_PRIORITY_IRA
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize with volatile reads to prevent optimization */
    asm volatile("" : "=r"(v1), "=r"(v2), "=r"(v3), "=r"(v4) : : "memory");
    asm volatile("" : "=r"(v5), "=r"(v6), "=r"(v7), "=r"(v8) : : "memory");
    asm volatile("" : "=r"(v9), "=r"(v10), "=r"(v11), "=r"(v12) : : "memory");
    asm volatile("" : "=r"(v13), "=r"(v14), "=r"(v15), "=r"(v16) : : "memory");
    
    /* Complex control flow to create intersecting live ranges */
    /* First conditional block - keep many variables live */
    if (v1 > 0) {
        /* Use all variables in computations */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v16;
        
        /* Clobber many ARM registers to increase pressure */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", 
                     "r6", "r7", "r8", "r9", "r10", "r12", "memory");
        
        /* More computations keeping variables live */
        v3 = v2 + v5;
        v6 = v8 * v11;
        v9 = v14 - v1;
        v12 = v4 ^ v7;
        v15 = v10 | v13;
    } else {
        /* Alternative path with different but overlapping live ranges */
        v4 = v1 + v2;
        v7 = v3 * v5;
        v10 = v6 - v8;
        v13 = v9 ^ v11;
        v16 = v12 | v14;
        
        /* Different clobber set */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", 
                     "r6", "r7", "r8", "memory");
        
        v2 = v4 + v7;
        v5 = v10 * v13;
        v8 = v16 - v1;
        v11 = v3 ^ v6;
        v14 = v9 | v12;
    }
    
    /* Second level of nesting - creates more complex CFG */
    if (v2 < v5) {
        /* Mix variables from both previous paths */
        v1 = v3 + v6;
        v4 = v7 * v9;
        v8 = v10 - v12;
        v13 = v14 ^ v15;
        
        /* Force spilling by clobbering all caller-saved regs */
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "r5", 
                     "r6", "r7", "r8", "r9", "r10", "r11", "r12", 
                     "r14", "memory");
        
        v3 = v1 + v4;
        v6 = v8 * v13;
        v9 = v2 - v5;
    } else {
        v5 = v1 + v4;
        v7 = v3 * v8;
        v10 = v6 - v9;
        
        asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", 
                     "r5", "r6", "memory");
        
        v1 = v5 + v7;
        v4 = v10 * v2;
    }
    
    /* Third conditional to create diamond CFG */
    switch (v1 & 3) {
        case 0:
            v2 = v3 + v4;
            v5 = v6 * v7;
            v8 = v9 - v10;
            asm volatile("" : : : "r0", "r1", "r2", "r3", "memory");
            break;
        case 1:
            v3 = v2 + v5;
            v6 = v8 * v9;
            v10 = v4 - v7;
            asm volatile("" : : : "r0", "r1", "r2", "memory");
            break;
        case 2:
            v4 = v3 + v6;
            v7 = v5 * v10;
            v9 = v2 - v8;
            asm volatile("" : : : "r0", "r1", "memory");
            break;
        default:
            v5 = v4 + v7;
            v8 = v3 * v9;
            v10 = v6 - v2;
            asm volatile("" : : : "r0", "memory");
            break;
    }
    
    /* Final use of all variables to extend live ranges */
    v16 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    /* Volatile write to prevent dead code elimination */
    asm volatile("" : : "r"(v16) : "memory");
}

/* Additional function with different pressure pattern */
TARGET_ARM FORCE_PRIORITY_IRA
void another_high_pressure_func(int param) {
    int a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize from parameter to create inter-block liveness */
    a1 = param;
    a2 = a1 * 2;
    a3 = a2 + 1;
    a4 = a3 - param;
    
    /* Loop to create back edges in CFG */
    for (int i = 0; i < 4; i++) {
        a5 = a1 + i;
        a6 = a2 * i;
        a7 = a3 - i;
        a8 = a4 ^ i;
        
        /* Nested condition inside loop */
        if (i & 1) {
            a9 = a5 + a6;
            a10 = a7 * a8;
            asm volatile("" : : : "r0", "r1", "r2", "r3", "r4", "memory");
        } else {
            a11 = a5 - a6;
            a12 = a7 ^ a8;
            asm volatile("" : : : "r0", "r1", "r2", "memory");
        }
        
        /* Use all variables to keep them live across iterations */
        a13 = a9 + a10 + a11 + a12;
        a14 = a13 * i;
        
        asm volatile("" : : "r"(a14) : "memory");
    }
    
    /* Force all variables to be live at exit */
    int result = a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13 + a14;
    asm volatile("" : : "r"(result) : "memory");
}

/* Main function exists only to make the file compilable */
int main(void) {
    /* Call high-pressure functions to ensure they're compiled */
    high_pressure_function();
    another_high_pressure_func(42);
    return 0;
}
