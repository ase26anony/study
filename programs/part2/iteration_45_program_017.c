/* Test program for GCC early rematerialization pass
 * Targets uncovered lines in early-remat.cc:930-937
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Main test function with high register pressure */
static int __attribute__((noinline)) test_remat(int seed) {
    /* Declare many variables to create register pressure */
    register int v0 asm ("r12") = seed;
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed ^ 0x1234;
    int v5 = seed | 0x5678;
    int v6 = seed & 0x9ABC;
    int v7 = ~seed;
    int v8 = seed << 2;
    int v9 = seed >> 1;
    int v10 = seed + 100;
    int v11 = seed - 50;
    int v12 = seed * 3;
    int v13 = seed / 2;
    int v14 = seed ^ 0xDEAD;
    int v15 = seed | 0xBEEF;
    int v16 = seed & 0xCAFE;
    int v17 = seed + 200;
    int v18 = seed - 100;
    int v19 = seed * 5;
    int v20 = seed / 4;
    int v21 = seed ^ 0xF00D;
    int v22 = seed | 0xBAAD;
    int v23 = seed & 0xFACE;
    
    /* Volatile variable to prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with many dependent operations
     * Creates long live ranges and def-use chains */
    for (int i = 0; i < 100; i++) {
        /* Mix register and stack variables in expressions */
        v0 = v1 + v2 + side_effect(i);
        v1 = v0 * v3 - v4;
        v2 = v1 | v5 ^ v6;
        v3 = v2 + v7 * v8;
        v4 = v3 / (v9 + 1) + v10;
        v5 = v4 ^ v11 & v12;
        v6 = v5 | v13 + v14;
        v7 = v6 * v15 - v16;
        v8 = v7 + v17 / (v18 + 1);
        v9 = v8 ^ v19 & v20;
        v10 = v9 | v21 + v22;
        v11 = v10 * v23 - v0;
        v12 = v11 + v1 * v2;
        v13 = v12 ^ v3 & v4;
        v14 = v13 | v5 + v6;
        v15 = v14 * v7 - v8;
        v16 = v15 + v9 / (v10 + 1);
        v17 = v16 ^ v11 & v12;
        v18 = v17 | v13 + v14;
        v19 = v18 * v15 - v16;
        v20 = v19 + v17 / (v18 + 1);
        v21 = v20 ^ v19 & v20;
        v22 = v21 | v21 + v22;
        v23 = v22 * v23 - v0;
        
        /* Use asm to clobber registers and increase pressure */
        asm volatile ("# Force register clobber" : : : 
            "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* Memory barrier to prevent reordering */
        barrier = i;
    }
    
    /* Conditional use to create different basic blocks */
    int result;
    if (v0 > 0) {
        result = v1 + v2 + v3 + v4 + v5;
        result = result * v6 - v7 + v8;
    } else {
        result = v9 + v10 + v11 + v12 + v13;
        result = result * v14 - v15 + v16;
    }
    
    /* More complex conditional with overlapping live ranges */
    for (int i = 0; i < 50; i++) {
        int temp = side_effect(i);
        if (temp & 1) {
            v17 = v18 + v19 * temp;
            v18 = v19 - v20 / (temp + 1);
        } else {
            v19 = v20 ^ v21 & temp;
            v20 = v21 | v22 + temp;
        }
        
        /* Cross-block value usage */
        result += (v17 > v19) ? v18 : v20;
        
        /* Another asm to create register constraints */
        asm volatile ("# Another clobber" : : : 
            "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23");
    }
    
    /* Final computation using all variables */
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    result += v10 + v11 + v12 + v13 + v14 + v15 + v16;
    result += v17 + v18 + v19 + v20 + v21 + v22 + v23;
    
    return result;
}

int main(void) {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        int result = test_remat(i * 12345);
        total += result;
        printf("Iteration %d: result = %d, total = %d\n", i, result, total);
    }
    
    /* Use result to prevent dead code elimination */
    if (total > 0) {
        printf("Final total: %d\n", total);
    } else {
        printf("Unexpected result\n");
    }
    
    return 0;
}
