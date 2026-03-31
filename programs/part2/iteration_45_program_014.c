/* Test program for GCC early rematerialization pass
 * Targeting lines 930-937 in early-remat.cc
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
    register int v0 asm ("r12") = seed + 1;
    register int v1 asm ("r13") = seed * 2;
    int v2 = seed - 5;
    int v3 = seed + 7;
    int v4 = seed * 3;
    int v5 = seed / 2;
    int v6 = seed + 11;
    int v7 = seed - 13;
    int v8 = seed * 5;
    int v9 = seed + 17;
    int v10 = seed - 19;
    int v11 = seed * 7;
    int v12 = seed + 23;
    int v13 = seed - 29;
    int v14 = seed * 11;
    int v15 = seed + 31;
    int v16 = seed - 37;
    int v17 = seed * 13;
    int v18 = seed + 41;
    int v19 = seed - 43;
    
    /* Volatile variable to prevent optimizations */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Create def-use chains across operations */
        v0 = v1 + v2 + side_effect(i);
        v3 = v0 * v4 - v5;
        v6 = v3 / (v7 + 1) + v8;
        v9 = v6 * v10 - v11;
        v12 = v9 + v13 * v14;
        v15 = v12 - v16 / (v17 + 1);
        v18 = v15 * v19 + v0;
        v1 = v18 - v2 * v3;
        v4 = v1 + v5 - v6;
        v7 = v4 * v8 / (v9 + 1);
        v10 = v7 - v11 + v12;
        v13 = v10 * v14 - v15;
        v16 = v13 + v17 * v18;
        v19 = v16 - v0 / (v1 + 1);
        v2 = v19 * v3 + v4;
        v5 = v2 - v6 * v7;
        v8 = v5 + v9 - v10;
        v11 = v8 * v12 / (v13 + 1);
        v14 = v11 - v15 + v16;
        v17 = v14 * v18 - v19;
        
        /* Use volatile to create memory side effects */
        barrier = i;
        
        /* Conditional to create different basic blocks */
        if (i & 1) {
            v0 = v1 + side_effect(v2);
            v3 = v4 * side_effect(v5);
        } else {
            v0 = v6 - side_effect(v7);
            v3 = v8 / (side_effect(v9) + 1);
        }
        
        /* More computations to extend live ranges */
        v12 = v0 * v3 + v1;
        v13 = v12 - v4 * v5;
        v14 = v13 + v6 / (v7 + 1);
        v15 = v14 * v8 - v9;
        v16 = v15 + v10 * v11;
        v17 = v16 - v12 / (v13 + 1);
        v18 = v17 * v14 + v15;
        v19 = v18 - v16 * v17;
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                 v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19;
    
    /* Inline asm to clobber registers and increase pressure */
    asm volatile ("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), "r"(v4),
                       "r"(v5), "r"(v6), "r"(v7), "r"(v8), "r"(v9),
                       "r"(v10), "r"(v11), "r"(v12), "r"(v13), "r"(v14),
                       "r"(v15), "r"(v16), "r"(v17), "r"(v18), "r"(v19) :
                       "memory");
    
    return result + barrier;
}

/* Another function to create interprocedural pressure */
static int __attribute__((noinline)) helper(int a, int b, int c) {
    /* Complex expression that might be rematerialized */
    int t1 = a * b + c;
    int t2 = b * c - a;
    int t3 = c * a + b;
    
    /* Volatile access */
    volatile int v = 0;
    v = t1 + t2 + t3;
    
    return (t1 * t2) / (t3 + 1) + v;
}

int main(void) {
    int total = 0;
    
    /* Call test function multiple times with different seeds
     * to prevent constant propagation */
    for (int i = 0; i < 50; i++) {
        int seed = i * 1234567;
        total += test_remat(seed);
        total += helper(seed, seed + 1, seed + 2);
        
        /* Prevent loop unrolling */
        if (total & 1) {
            total += 1;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
