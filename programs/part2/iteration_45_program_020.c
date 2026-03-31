/* Test program for GCC early rematerialization pass
 * Targets uncovered lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlineable helper to create side effects and prevent optimization */
static int __attribute__((noinline)) side_effect(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Main test function creating register pressure */
static int __attribute__((noinline)) test_remat_pressure(int seed) {
    /* Declare many variables to create pseudo registers */
    register int r0 asm ("r12") = seed;
    int v1 = seed * 2;
    int v2 = seed + 1;
    int v3 = seed - 5;
    int v4 = seed ^ 0x55;
    int v5 = seed * 3;
    int v6 = seed / 2;
    int v7 = seed << 1;
    int v8 = seed >> 2;
    int v9 = seed | 0xFF;
    int v10 = seed & 0xF0;
    int v11 = seed + 100;
    int v12 = seed - 50;
    int v13 = ~seed;
    int v14 = seed * seed;
    int v15 = seed + 1000;
    int v16 = seed * 7;
    int v17 = seed % 13;
    int v18 = seed + 777;
    int v19 = seed * 11;
    int v20 = seed ^ 0xAA;
    
    /* Volatile variable to create memory side effects */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Force pseudo register usage across iterations */
        r0 = side_effect(r0 + v1);
        v1 = v2 + v3 + barrier;
        v2 = v4 * v5 - r0;
        v3 = v6 ^ v7 | v1;
        v4 = v8 + v9 * v2;
        v5 = v10 - v11 + v3;
        v6 = v12 & v13 | v4;
        v7 = v14 / (v5 + 1);
        v8 = v15 ^ v16 + v6;
        v9 = v17 * v18 - v7;
        v10 = v19 + v20 * v8;
        v11 = r0 + v1 + v9;
        v12 = v2 - v3 + v10;
        v13 = v4 * v5 ^ v11;
        v14 = v6 + v7 | v12;
        v15 = v8 - v9 & v13;
        v16 = v10 * v11 + v14;
        v17 = v12 ^ v13 - v15;
        v18 = v14 | v15 * v16;
        v19 = v16 + v17 + v18;
        v20 = v18 - v19 + r0;
        
        /* Memory barrier to prevent reordering */
        barrier = i;
        
        /* Conditional to create different basic blocks */
        if (i & 1) {
            r0 = v1 + v2;
            v3 = v4 * v5;
        } else {
            r0 = v6 - v7;
            v8 = v9 ^ v10;
        }
        
        /* Use results in next iteration to maintain liveness */
        v1 = v1 + (v20 & 1);
        v2 = v2 ^ (r0 & 0xFF);
    }
    
    /* Mix all results to ensure they're used */
    int result = r0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return result;
}

/* Additional complexity with inline asm to affect register allocation */
static int __attribute__((noinline)) test_with_asm(int x) {
    int a = x * 2;
    int b = x + 3;
    int c = x - 4;
    int d = x ^ 0x33;
    
    /* Inline asm that clobbers registers */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=r" (a)
        : "r" (b), "r" (c)
        : "%eax", "cc"
    );
    
    /* More computations to increase pressure */
    for (int i = 0; i < 50; i++) {
        a = a + b + side_effect(i);
        b = b * c - a;
        c = c ^ d + b;
        d = d - a * c;
        
        /* Force spill/reload candidates */
        if (i % 3 == 0) {
            int t1 = a * b;
            int t2 = c + d;
            int t3 = t1 ^ t2;
            a = t3 + side_effect(t1);
        }
    }
    
    return a + b + c + d;
}

int main(void) {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        total += test_remat_pressure(i * 12345);
        total += test_with_asm(i * 54321);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
