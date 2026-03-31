/* early-remat-trigger.c
 * Designed to trigger virtual register creation in GCC's early rematerialization pass
 * Compile with: gcc -O2 -fno-schedule-insns -fno-schedule-insns2 -fno-peephole2 early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to prevent optimization */
static int __attribute__((noinline)) helper(int x) {
    static volatile int counter = 0;
    counter++;
    return x + (counter & 1);
}

/* Main test function designed to create high register pressure */
static int __attribute__((noinline)) test_remat(int seed) {
    /* Declare many variables to create pseudo registers */
    register int v0 asm ("r12") = seed;
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed / 3;
    int v4 = seed ^ 0x55;
    int v5 = seed | 0xAA;
    int v6 = seed & 0xF0;
    int v7 = ~seed;
    int v8 = seed << 2;
    int v9 = seed >> 1;
    int v10 = seed + 100;
    int v11 = seed - 50;
    int v12 = seed * 3;
    int v13 = seed / 2;
    int v14 = seed ^ 0xFF;
    int v15 = seed | 0x0F;
    int v16 = seed & 0x0F;
    int v17 = seed + 200;
    int v18 = seed - 100;
    int v19 = seed * 5;
    int v20 = seed / 4;
    int v21 = seed ^ 0xAA;
    int v22 = seed | 0x55;
    int v23 = seed & 0x33;
    int v24 = seed + 300;
    int v25 = seed - 150;
    int v26 = seed * 7;
    int v27 = seed / 5;
    int v28 = seed ^ 0xCC;
    int v29 = seed | 0x33;
    
    /* Volatile variable to create side effects and prevent reordering */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < 100; i++) {
        /* Use helper to prevent CSE */
        int h = helper(i);
        
        /* Chain of dependent operations creating def-use chains */
        v0 = v0 + v1 + h;
        v1 = v1 ^ v2 ^ barrier;
        v2 = v2 * v3 + h;
        v3 = v3 | v4 | barrier;
        v4 = v4 & v5 & h;
        v5 = v5 + v6 + barrier;
        v6 = v6 - v7 - h;
        v7 = v7 ^ v8 ^ barrier;
        v8 = v8 * v9 + h;
        v9 = v9 | v10 | barrier;
        v10 = v10 & v11 & h;
        v11 = v11 + v12 + barrier;
        v12 = v12 - v13 - h;
        v13 = v13 ^ v14 ^ barrier;
        v14 = v14 * v15 + h;
        v15 = v15 | v16 | barrier;
        v16 = v16 & v17 & h;
        v17 = v17 + v18 + barrier;
        v18 = v18 - v19 - h;
        v19 = v19 ^ v20 ^ barrier;
        v20 = v20 * v21 + h;
        v21 = v21 | v22 | barrier;
        v22 = v22 & v23 & h;
        v23 = v23 + v24 + barrier;
        v24 = v24 - v25 - h;
        v25 = v25 ^ v26 ^ barrier;
        v26 = v26 * v27 + h;
        v27 = v27 | v28 | barrier;
        v28 = v28 & v29 & h;
        v29 = v29 + v0 + barrier;
        
        /* Conditional to create different basic blocks */
        if (i & 1) {
            /* Different computation path */
            v0 = v0 ^ v29;
            v1 = v1 | v28;
            barrier = i;  /* Side effect */
        } else {
            /* Alternative path */
            v29 = v29 & v0;
            v28 = v28 | v1;
            barrier = i + 1;  /* Side effect */
        }
        
        /* Inline asm to clobber registers and increase pressure */
        asm volatile ("" : : "r"(v0), "r"(v1), "r"(v2), "r"(v3), 
                      "r"(v4), "r"(v5), "r"(v6), "r"(v7) : 
                      "memory");
    }
    
    /* Final computation using all variables */
    int result = v0 + v1 - v2 + v3 - v4 + v5 - v6 + v7 - v8 + v9
                 - v10 + v11 - v12 + v13 - v14 + v15 - v16 + v17
                 - v18 + v19 - v20 + v21 - v22 + v23 - v24 + v25
                 - v26 + v27 - v28 + v29;
    
    /* Use result to prevent dead code elimination */
    asm volatile ("" : "+r"(result) : : "memory");
    return result;
}

int main() {
    int total = 0;
    
    /* Call with different seeds to prevent constant propagation */
    for (int i = 0; i < 10; i++) {
        int result = test_remat(i * 12345);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
    }
    
    printf("Total: %d\n", total);
    return total != 0 ? 0 : 1;
}
