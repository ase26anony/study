/* test-mcf-debug.c
 * Designed to trigger debug dumping of IRA's min-cost flow fixup graph
 * when compiled with GCC built with --enable-checking (MCF_DEBUG defined).
 * The program's runtime behavior is irrelevant; coverage occurs during
 * compilation within GCC's register allocator.
 */

/* Force use of priority-based register allocator for this function */
void __attribute__((optimize("O2", "-fira-algorithm=priority")))
__attribute__((target("arch=armv7-a")))
high_pressure_function(void)
{
    /* Declare many integer variables to create high register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16;
    
    /* Initialize all variables with different values to prevent optimization */
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5; v6 = 6; v7 = 7; v8 = 8;
    v9 = 9; v10 = 10; v11 = 11; v12 = 12; v13 = 13; v14 = 14; v15 = 15; v16 = 16;
    
    /* Clobber many ARM registers to increase perceived pressure */
    asm volatile("" : : : "memory", 
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                 "r8", "r9", "r10", "r11", "r12");
    
    /* Complex control flow with many live ranges across blocks */
    /* Block 1: Use all variables in computations */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v10 = v11 ^ v12;
    v13 = v14 | v15;
    v16 = v1 + v4;
    
    /* Force conditional branching with different variable usage patterns */
    if (v1 > v2) {
        /* Block 2: Different subset of computations */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v16;
        v1 = v2 + v5;
        
        /* More clobbering */
        asm volatile("" : : : "memory");
    } else {
        /* Block 3: Another subset */
        v3 = v4 + v5;
        v6 = v7 * v8;
        v9 = v10 - v11;
        v12 = v13 ^ v14;
        v15 = v16 | v1;
        v2 = v3 + v6;
    }
    
    /* Nested conditionals to create more complex CFG */
    switch (v1 & 0x3) {
        case 0:
            v4 = v5 + v6 + v7 + v8;
            v9 = v10 * v11 * v12;
            break;
        case 1:
            v5 = v6 + v7 + v8 + v9;
            v10 = v11 * v12 * v13;
            break;
        case 2:
            v6 = v7 + v8 + v9 + v10;
            v11 = v12 * v13 * v14;
            break;
        default:
            v7 = v8 + v9 + v10 + v11;
            v12 = v13 * v14 * v15;
            break;
    }
    
    /* Final computations using all variables to keep them live */
    v1 = v1 + v2 + v3 + v4;
    v5 = v5 + v6 + v7 + v8;
    v9 = v9 + v10 + v11 + v12;
    v13 = v13 + v14 + v15 + v16;
    
    /* Final clobber */
    asm volatile("" : : : "memory", 
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7");
    
    /* Use all results to prevent dead code elimination */
    asm volatile("" : : "r"(v1), "r"(v5), "r"(v9), "r"(v13));
}

/* Alternative x86 version for testing on x86 hosts */
void __attribute__((optimize("O2", "-fira-algorithm=priority")))
__attribute__((target("arch=pentium4")))  /* Older x86 with fewer registers */
high_pressure_function_x86(void)
{
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14;
    
    v1 = 1; v2 = 2; v3 = 3; v4 = 4; v5 = 5; v6 = 6; v7 = 7;
    v8 = 8; v9 = 9; v10 = 10; v11 = 11; v12 = 12; v13 = 13; v14 = 14;
    
    /* Clobber x86 registers */
    asm volatile("" : : : "memory",
                 "eax", "ebx", "ecx", "edx", "esi", "edi");
    
    /* Similar complex control flow */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v10 = v11 ^ v12;
    v13 = v14 | v1;
    
    if (v1 > v2) {
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
    } else {
        v3 = v4 + v5;
        v6 = v7 * v8;
        v9 = v10 - v11;
        v12 = v13 ^ v14;
    }
    
    /* Force more register pressure with loop */
    for (int i = 0; i < 3; i++) {
        v1 = v1 + v2;
        v3 = v3 + v4;
        v5 = v5 + v6;
        v7 = v7 + v8;
        v9 = v9 + v10;
        v11 = v11 + v12;
        v13 = v13 + v14;
    }
    
    asm volatile("" : : "r"(v1), "r"(v3), "r"(v5), "r"(v7), 
                       "r"(v9), "r"(v11), "r"(v13));
}

/* Main exists only to make the file compilable */
int main(void)
{
    return 0;
}
