/* Test case for GCC selective scheduling RTL dump coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 128
#define ITERS 100000

/* Target function with complex scheduling requirements */
static __attribute__((noinline)) 
int stress_sched(int seed) {
    volatile int barrier; /* Force memory operations */
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    int i, j, sum = 0;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = (i * seed) & 0xFF;
        arr2[i] = (i + seed) & 0xFF;
        farr1[i] = (i * 0.1f) + seed;
        farr2[i] = (i * 0.2f) - seed;
    }
    
    /* Outer loop - provides iteration count */
    for (j = 0; j < ITERS; j++) {
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < SIZE - 1; i++) {
            /* Chain of dependent integer operations */
            int t1 = arr1[i-1] * 3;
            int t2 = arr1[i] * 5;
            int t3 = arr1[i+1] * 7;
            int t4 = t1 + t2 + t3 + seed;
            
            /* Floating-point operations mixed in */
            float ft1 = farr1[i-1] * 1.5f;
            float ft2 = farr1[i] * 2.5f;
            float ft3 = ft1 + ft2 + (j * 0.01f);
            
            /* Volatile read creates scheduling barrier */
            barrier = arr2[i];
            
            /* Conditional with side effects in both branches */
            if ((t4 ^ barrier) & 0x40) {
                /* Branch 1: complex calculation chain */
                int b1 = t4 * 11;
                int b2 = b1 - (barrier << 3);
                int b3 = b2 ^ (t4 >> 2);
                arr2[i] = b3;
                
                /* More FP ops in this branch */
                farr2[i] = ft3 * 3.14f + barrier;
            } else {
                /* Branch 2: different calculation chain */
                int b1 = t4 * 13;
                int b2 = b1 + (barrier << 2);
                int b3 = b2 | (t4 << 1);
                arr2[i] = b3;
                
                /* Different FP ops */
                farr2[i] = ft3 * 2.71f - barrier;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Late use of early-computed values - extends live ranges */
            int late_use = t4 + (arr2[i] & 0xF);
            float late_fuse = ft3 + farr2[i] * 0.5f;
            
            /* More operations creating register pressure */
            arr1[i] = (arr1[i] * 2) - late_use;
            farr1[i] = farr1[i] + late_fuse - j;
            
            /* Another volatile write */
            barrier = late_use;
        }
        
        /* Cross-iteration dependency */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Reduce results to prevent elimination */
        if (j % 100 == 0) {
            for (i = 0; i < SIZE; i++) {
                sum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i];
            }
        }
    }
    
    return sum;
}

int main() {
    int total = 0;
    int i;
    
    /* Multiple calls with different seeds */
    for (i = 0; i < 5; i++) {
        int result = stress_sched(i * 100);
        total += result;
        printf("Iteration %d: result = %d\n", i, result);
    }
    
    printf("Total checksum: %d\n", total);
    
    /* Use result to prevent dead code elimination */
    if (total == 0) {
        printf("Zero result - unexpected\n");
    }
    
    return 0;
}
