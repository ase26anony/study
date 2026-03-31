/* test-mcf-debug.c
 * Compile with: gcc-debug -O2 -fira-algorithm=priority -c test-mcf-debug.c -o test.o
 * For ARM: gcc-debug -O2 -fira-algorithm=priority -march=armv7-a -c test-mcf-debug.c
 */

/* Force priority-based IRA algorithm */
#ifdef __GNUC__
#define FORCE_PRIORITY_IRA __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define FORCE_PRIORITY_IRA
#endif

/* Target ARM for fewer registers */
#ifdef __GNUC__
#define TARGET_ARM __attribute__((target("arch=armv7-a")))
#else
#define TARGET_ARM
#endif

/* Function to create massive register pressure */
FORCE_PRIORITY_IRA TARGET_ARM
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize with volatile values to prevent constant propagation */
    asm volatile("" : "=r"(v0) : "0"(1));
    asm volatile("" : "=r"(v1) : "0"(2));
    asm volatile("" : "=r"(v2) : "0"(3));
    asm volatile("" : "=r"(v3) : "0"(4));
    asm volatile("" : "=r"(v4) : "0"(5));
    asm volatile("" : "=r"(v5) : "0"(6));
    asm volatile("" : "=r"(v6) : "0"(7));
    asm volatile("" : "=r"(v7) : "0"(8));
    asm volatile("" : "=r"(v8) : "0"(9));
    asm volatile("" : "=r"(v9) : "0"(10));
    asm volatile("" : "=r"(v10) : "0"(11));
    asm volatile("" : "=r"(v11) : "0"(12));
    asm volatile("" : "=r"(v12) : "0"(13));
    asm volatile("" : "=r"(v13) : "0"(14));
    asm volatile("" : "=r"(v14) : "0"(15));
    asm volatile("" : "=r"(v15) : "0"(16));
    
    /* Complex control flow to create intersecting live ranges */
    int selector = v0;
    
    /* First block: use many variables together */
    v1 = v2 + v3;
    v4 = v5 * v6;
    v7 = v8 - v9;
    v10 = v11 ^ v12;
    v13 = v14 | v15;
    
    /* Clobber many registers to increase pressure */
    asm volatile("" : : : "memory", 
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", 
                 "r8", "r9", "r10", "r11", "r12", "r14");
    
    /* Multi-way branch creating different live range patterns */
    if (selector > 8) {
        /* Path A: different variable usage pattern */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v0;
        
        /* More computations to extend live ranges */
        v1 = v2 * v3;
        v4 = v5 + v6;
        v7 = v8 ^ v9;
        v10 = v11 - v12;
        v13 = v14 | v15;
    } else if (selector > 4) {
        /* Path B: yet another pattern */
        v3 = v4 + v5;
        v6 = v7 * v8;
        v9 = v10 - v11;
        v12 = v13 ^ v14;
        v15 = v0 | v1;
        
        v2 = v3 * v4;
        v5 = v6 + v7;
        v8 = v9 ^ v10;
        v11 = v12 - v13;
        v14 = v15 | v0;
    } else {
        /* Path C: third pattern */
        v4 = v5 + v6;
        v7 = v8 * v9;
        v10 = v11 - v12;
        v13 = v14 ^ v15;
        v0 = v1 | v2;
        
        v3 = v4 * v5;
        v6 = v7 + v8;
        v9 = v10 ^ v11;
        v12 = v13 - v14;
        v15 = v0 | v1;
    }
    
    /* Merge point: force all variables live again */
    v0 = v1 + v2;
    v3 = v4 * v5;
    v6 = v7 - v8;
    v9 = v10 ^ v11;
    v12 = v13 | v14;
    
    /* Second clobber to maintain pressure */
    asm volatile("" : : : "memory",
                 "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                 "r8", "r9", "r10", "r11", "r12");
    
    /* Nested control flow for additional complexity */
    for (int i = 0; i < 3; i++) {
        int t = v0 + i;
        if (t & 1) {
            v1 = v2 * v3;
            v4 = v5 + v6;
        } else {
            v7 = v8 - v9;
            v10 = v11 ^ v12;
        }
        
        /* Rotate values to create data dependencies */
        int tmp = v15;
        v15 = v14;
        v14 = v13;
        v13 = v12;
        v12 = v11;
        v11 = v10;
        v10 = v9;
        v9 = v8;
        v8 = v7;
        v7 = v6;
        v6 = v5;
        v5 = v4;
        v4 = v3;
        v3 = v2;
        v2 = v1;
        v1 = tmp;
    }
    
    /* Final use of all variables to ensure they're live at end */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3),
                       "r"(v4), "r"(v5), "r"(v6), "r"(v7),
                       "r"(v8), "r"(v9), "r"(v10), "r"(v11),
                       "r"(v12), "r"(v13), "r"(v14), "r"(v15));
}

/* Additional high-pressure function with different pattern */
FORCE_PRIORITY_IRA TARGET_ARM
void another_high_pressure_function(void) {
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14;
    
    /* Initialize */
    asm volatile("mov %0, #1" : "=r"(a0));
    asm volatile("mov %0, #2" : "=r"(a1));
    asm volatile("mov %0, #3" : "=r"(a2));
    asm volatile("mov %0, #4" : "=r"(a3));
    asm volatile("mov %0, #5" : "=r"(a4));
    asm volatile("mov %0, #6" : "=r"(a5));
    asm volatile("mov %0, #7" : "=r"(a6));
    asm volatile("mov %0, #8" : "=r"(a7));
    asm volatile("mov %0, #9" : "=r"(a8));
    asm volatile("mov %0, #10" : "=r"(a9));
    asm volatile("mov %0, #11" : "=r"(a10));
    asm volatile("mov %0, #12" : "=r"(a11));
    asm volatile("mov %0, #13" : "=r"(a12));
    asm volatile("mov %0, #14" : "=r"(a13));
    asm volatile("mov %0, #15" : "=r"(a14));
    
    /* Switch statement for complex control flow */
    int control = a0;
    switch (control) {
        case 1:
            a1 = a2 + a3;
            a4 = a5 * a6;
            a7 = a8 - a9;
            a10 = a11 ^ a12;
            a13 = a14 | a0;
            break;
        case 2:
            a2 = a3 + a4;
            a5 = a6 * a7;
            a8 = a9 - a10;
            a11 = a12 ^ a13;
            a14 = a0 | a1;
            break;
        case 3:
            a3 = a4 + a5;
            a6 = a7 * a8;
            a9 = a10 - a11;
            a12 = a13 ^ a14;
            a0 = a1 | a2;
            break;
        default:
            a4 = a5 + a6;
            a7 = a8 * a9;
            a10 = a11 - a12;
            a13 = a14 ^ a0;
            a1 = a2 | a3;
            break;
    }
    
    /* Force all live at exit */
    asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3),
                       "r"(a4), "r"(a5), "r"(a6), "r"(a7),
                       "r"(a8), "r"(a9), "r"(a10), "r"(a11),
                       "r"(a12), "r"(a13), "r"(a14));
}

/* Main function exists only to make the file compilable */
int main(void) {
    /* The functions are never called - coverage happens at compile time */
    return 0;
}
