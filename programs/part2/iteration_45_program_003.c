/* early-remat-trigger.c
 * Designed to trigger uncovered lines in GCC's early rematerialization pass
 * Specifically targets lines 930-937 in early-remat.cc
 */

#include <stdio.h>
#include <stdint.h>

/* Non-inlineable helper to prevent optimization */
static int __attribute__((noinline)) compute_offset(int x) {
    volatile int dummy = 0;
    return x + (dummy ? 0 : 1);
}

/* Function to create register pressure and trigger rematerialization */
static int __attribute__((noinline)) trigger_remat(int iterations) {
    /* Declare many variables to create register pressure */
    register int v0 asm ("r12") = iterations;
    register int v1 asm ("r13") = iterations * 2;
    int v2 = iterations + 1;
    int v3 = iterations * 3;
    int v4 = iterations - 1;
    int v5 = iterations / 2;
    int v6 = iterations % 7;
    int v7 = iterations << 1;
    int v8 = iterations >> 1;
    int v9 = iterations ^ 0x55;
    int v10 = iterations | 0xAA;
    int v11 = iterations & 0xFF;
    int v12 = ~iterations;
    int v13 = iterations + 100;
    int v14 = iterations - 50;
    int v15 = iterations * 5;
    int v16 = iterations / 3;
    int v17 = iterations % 11;
    int v18 = iterations << 2;
    int v19 = iterations >> 2;
    int v20 = iterations ^ 0x33;
    int v21 = iterations | 0xCC;
    int v22 = iterations & 0xF0;
    int v23 = ~iterations + 1;
    int v24 = iterations + 200;
    int v25 = iterations - 75;
    int v26 = iterations * 7;
    int v27 = iterations / 4;
    int v28 = iterations % 13;
    int v29 = iterations << 3;
    
    /* Volatile variable to create side effects and prevent optimization */
    volatile int barrier = 0;
    
    /* Complex computation with overlapping live ranges */
    for (int i = 0; i < iterations; i++) {
        /* Create def-use chains across operations */
        v0 = v1 + v2 + compute_offset(i);
        v1 = v0 * v3 - compute_offset(v0);
        v2 = v1 / (v4 + 1) + compute_offset(v1);
        v3 = v2 ^ v5 ^ compute_offset(v2);
        v4 = v3 | v6 | compute_offset(v3);
        v5 = v4 & v7 & compute_offset(v4);
        v6 = ~v5 + compute_offset(v5);
        v7 = v6 << (v8 % 4) + compute_offset(v6);
        v8 = v7 >> (v9 % 4) + compute_offset(v7);
        
        v9 = v10 + v11 + compute_offset(v8);
        v10 = v9 * v12 - compute_offset(v9);
        v11 = v10 / (v13 + 1) + compute_offset(v10);
        v12 = v11 ^ v14 ^ compute_offset(v11);
        v13 = v12 | v15 | compute_offset(v12);
        v14 = v13 & v16 & compute_offset(v13);
        v15 = ~v14 + compute_offset(v14);
        v16 = v15 << (v17 % 4) + compute_offset(v15);
        v17 = v16 >> (v18 % 4) + compute_offset(v16);
        
        v18 = v19 + v20 + compute_offset(v17);
        v19 = v18 * v21 - compute_offset(v18);
        v20 = v19 / (v22 + 1) + compute_offset(v19);
        v21 = v20 ^ v23 ^ compute_offset(v20);
        v22 = v21 | v24 | compute_offset(v21);
        v23 = v22 & v25 & compute_offset(v22);
        v24 = ~v23 + compute_offset(v23);
        v25 = v24 << (v26 % 4) + compute_offset(v24);
        v26 = v25 >> (v27 % 4) + compute_offset(v25);
        
        v27 = v28 + v29 + compute_offset(v26);
        v28 = v27 * v0 - compute_offset(v27);
        v29 = v28 / (v1 + 1) + compute_offset(v28);
        
        /* Use asm to create register clobbering and increase pressure */
        asm volatile ("# Force register pressure" : 
                     : "r" (v0), "r" (v1), "r" (v2), "r" (v3),
                       "r" (v4), "r" (v5), "r" (v6), "r" (v7));
        
        /* Memory barrier to prevent reordering */
        barrier = i;
    }
    
    /* Conditional use of variables to maintain liveness */
    int result = 0;
    if (v0 > 0) result += v1;
    if (v2 > 0) result += v3;
    if (v4 > 0) result += v5;
    if (v6 > 0) result += v7;
    if (v8 > 0) result += v9;
    if (v10 > 0) result += v11;
    if (v12 > 0) result += v13;
    if (v14 > 0) result += v15;
    if (v16 > 0) result += v17;
    if (v18 > 0) result += v19;
    if (v20 > 0) result += v21;
    if (v22 > 0) result += v23;
    if (v24 > 0) result += v25;
    if (v26 > 0) result += v27;
    if (v28 > 0) result += v29;
    
    /* Final computation that uses all variables */
    result = (result + barrier) % 1000;
    
    return result;
}

/* Main function to drive the test */
int main() {
    int total = 0;
    
    /* Call with different parameters to prevent constant propagation */
    for (int i = 1; i <= 10; i++) {
        int result = trigger_remat(50 + i * 10);
        total += result;
        printf("Iteration %d: result = %d, total = %d\n", i, result, total);
    }
    
    /* Additional test with different patterns */
    for (int i = 1; i <= 5; i++) {
        int result = trigger_remat(100 + i * 20);
        total += result;
        printf("Extra %d: result = %d, total = %d\n", i, result, total);
    }
    
    printf("Final total: %d\n", total);
    return total > 0 ? 0 : 1;
}
