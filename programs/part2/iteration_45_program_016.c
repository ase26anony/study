/* Test program for GCC early rematerialization pass
 * Targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter & 1;
}

/* Main test function with high register pressure */
static int __attribute__((noinline)) test_remat(int iterations) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = iterations;
    int v1 = v0 + 1;
    int v2 = v1 * 2;
    int v3 = v2 - v0;
    int v4 = v3 << 1;
    int v5 = v4 | 0xFF;
    int v6 = v5 ^ v1;
    int v7 = v6 + v2;
    int v8 = v7 * 3;
    int v9 = v8 / 2;
    int v10 = v9 & 0x7F;
    int v11 = v10 + v3;
    int v12 = v11 * v4;
    int v13 = v12 - v5;
    int v14 = v13 >> 1;
    int v15 = v14 ^ v6;
    int v16 = v15 + v7;
    int v17 = v16 * 5;
    int v18 = v17 % 7;
    int v19 = v18 | v8;
    int v20 = v19 ^ v9;
    int v21 = v20 + v10;
    int v22 = v21 * v11;
    int v23 = v22 - v12;
    int v24 = v23 >> 2;
    int v25 = v24 ^ v13;
    int v26 = v25 + v14;
    int v27 = v26 * 11;
    int v28 = v27 % 13;
    int v29 = v28 | v15;
    int v30 = v29 ^ v16;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex loop with dependent computations */
    for (int i = 0; i < iterations; i++) {
        /* Mix in side effects to create non-movable instructions */
        int se = side_effect(i);
        
        /* Chain of dependent operations using all variables */
        v0 = v30 + se;
        v1 = v0 * v29;
        v2 = v1 - v28;
        v3 = v2 << (se + 1);
        v4 = v3 | v27;
        v5 = v4 ^ v26;
        v6 = v5 + v25;
        v7 = v6 * v24;
        v8 = v7 - v23;
        v9 = v8 >> (se + 2);
        v10 = v9 ^ v22;
        v11 = v10 + v21;
        v12 = v11 * v20;
        v13 = v12 - v19;
        v14 = v13 << (se + 3);
        v15 = v14 | v18;
        v16 = v15 ^ v17;
        v17 = v16 + v16;
        v18 = v17 * v15;
        v19 = v18 - v14;
        v20 = v19 >> (se + 4);
        v21 = v20 ^ v13;
        v22 = v21 + v12;
        v23 = v22 * v11;
        v24 = v23 - v10;
        v25 = v24 << (se + 5);
        v26 = v25 | v9;
        v27 = v26 ^ v8;
        v28 = v27 + v7;
        v29 = v28 * v6;
        v30 = v29 - v5;
        
        /* Memory barrier to prevent reordering */
        barrier = i;
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 +
                 v20 + v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    return result & 0xFFFF; /* Return meaningful result */
}

/* Inline asm to clobber registers and increase pressure */
static void clobber_regs(void) {
    asm volatile (
        "mov r0, r0\n\t"
        "mov r1, r1\n\t"
        "mov r2, r2\n\t"
        "mov r3, r3\n\t"
        "mov r4, r4\n\t"
        "mov r5, r5\n\t"
        "mov r6, r6\n\t"
        "mov r7, r7\n\t"
        "mov r8, r8\n\t"
        "mov r9, r9\n\t"
        "mov r10, r10\n\t"
        "mov r11, r11\n\t"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11"
    );
}

int main(void) {
    int total = 0;
    
    /* Call test function with different parameters */
    for (int i = 1; i <= 10; i++) {
        clobber_regs(); /* Force register spills */
        int result = test_remat(i * 3);
        total += result;
        printf("Iteration %d: result = %d, total = %d\n", i, result, total);
    }
    
    return total != 0 ? 0 : 1;
}
