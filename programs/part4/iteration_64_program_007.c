/* Test program to trigger min-cost flow fixup graph debug output in GCC IRA.
   Compile with a debug-built GCC (configured with --enable-checking) using:
     gcc-debug -O2 -fira-algorithm=priority -c this_file.c -o this_file.o
   For ARM targets (fewer registers, more pressure):
     gcc-debug -O2 -fira-algorithm=priority -march=armv7-a -c this_file.c
*/

/* Force use of priority-based allocator which uses min-cost flow solver */
#ifdef __GNUC__
#define PRIORITY_ALLOC __attribute__((optimize("O2", "-fira-algorithm=priority")))
#else
#define PRIORITY_ALLOC
#endif

/* Target ARM for fewer general-purpose registers */
#ifdef __GNUC__
#define ARM_TARGET __attribute__((target("arch=armv7-a")))
#else
#define ARM_TARGET
#endif

/* Function designed to create maximum register pressure */
PRIORITY_ALLOC ARM_TARGET
void high_pressure_function(void) {
    /* Declare many integer variables to create live ranges */
    int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15;
    
    /* Initialize all variables with different values to prevent optimization */
    v0 = 0;
    v1 = 1;
    v2 = 2;
    v3 = 3;
    v4 = 4;
    v5 = 5;
    v6 = 6;
    v7 = 7;
    v8 = 8;
    v9 = 9;
    v10 = 10;
    v11 = 11;
    v12 = 12;
    v13 = 13;
    v14 = 14;
    v15 = 15;
    
    /* Complex control flow to create many basic blocks */
    /* First level of branching */
    if (v0) {
        /* Use all variables in this block to keep them live */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 - v9;
        v10 = v11 ^ v12;
        v13 = v14 | v15;
        
        /* Clobber many registers to increase pressure */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
                     "r6", "r7", "r8", "r9", "r10", "r11", "r12");
    } else {
        /* Different computation pattern to create different live ranges */
        v2 = v3 + v4;
        v5 = v6 * v7;
        v8 = v9 - v10;
        v11 = v12 ^ v13;
        v14 = v15 | v0;
        
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5",
                     "r6", "r7", "r8", "r9", "r10", "r11", "r12");
    }
    
    /* Second level of branching - nested to create more complex CFG */
    if (v1) {
        if (v2) {
            v3 = v4 + v5;
            v6 = v7 * v8;
            v9 = v10 - v11;
            v12 = v13 ^ v14;
            v15 = v0 | v1;
        } else {
            v4 = v5 + v6;
            v7 = v8 * v9;
            v10 = v11 - v12;
            v13 = v14 ^ v15;
            v0 = v1 | v2;
        }
        
        /* More register clobbering */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5",
                     "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14");
    }
    
    /* Third level - switch statement for more basic blocks */
    switch (v3 & 3) {
        case 0:
            v4 = v5 + v6 + v7;
            v8 = v9 * v10 * v11;
            break;
        case 1:
            v5 = v6 + v7 + v8;
            v9 = v10 * v11 * v12;
            break;
        case 2:
            v6 = v7 + v8 + v9;
            v10 = v11 * v12 * v13;
            break;
        case 3:
            v7 = v8 + v9 + v10;
            v11 = v12 * v13 * v14;
            break;
    }
    
    /* Final computations using all variables to ensure they remain live */
    v0 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    v1 = v0 * 2;
    v2 = v1 / 3;
    v3 = v2 % 5;
    v4 = v3 << 2;
    v5 = v4 >> 1;
    v6 = v5 & 0xFF;
    v7 = v6 | 0x80;
    v8 = v7 ^ 0x55;
    
    /* Final clobber to prevent optimization */
    asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5",
                 "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r14");
    
    /* Use all variables one more time */
    asm volatile("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4), 
                 "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
                 "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15));
}

/* Second high-pressure function with different pattern */
PRIORITY_ALLOC ARM_TARGET
void another_high_pressure_function(int param) {
    int a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13;
    
    /* Initialize from parameter to create different live ranges */
    a0 = param;
    a1 = a0 + 1;
    a2 = a1 * 2;
    a3 = a2 - 3;
    a4 = a3 / 4;
    a5 = a4 % 5;
    a6 = a5 << 1;
    a7 = a6 >> 2;
    a8 = a7 & 0xF;
    a9 = a8 | 0x10;
    a10 = a9 ^ 0x20;
    a11 = a10 + a0;
    a12 = a11 * a1;
    a13 = a12 - a2;
    
    /* Loop to create more register pressure */
    for (int i = 0; i < 4; i++) {
        /* Rotate values to create complex live ranges */
        int tmp = a0;
        a0 = a1;
        a1 = a2;
        a2 = a3;
        a3 = a4;
        a4 = a5;
        a5 = a6;
        a6 = a7;
        a7 = a8;
        a8 = a9;
        a9 = a10;
        a10 = a11;
        a11 = a12;
        a12 = a13;
        a13 = tmp;
        
        /* Clobber registers in loop */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5",
                     "r6", "r7", "r8", "r9", "r10", "r11", "r12");
    }
    
    /* Use all variables at the end */
    asm volatile("" : : "r"(a0), "r"(a1), "r"(a2), "r"(a3), "r"(a4), 
                 "r"(a5), "r"(a6), "r"(a7), "r"(a8), "r"(a9), "r"(a10),
                 "r"(a11), "r"(a12), "r"(a13));
}

/* Main function exists only to make the file compilable */
int main(void) {
    high_pressure_function();
    another_high_pressure_function(42);
    return 0;
}
