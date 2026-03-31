#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is crafted to trigger GCC's modulo scheduler debugging output
 * specifically targeting the uncovered lines in modulo-sched.cc
 */

#define SIZE 1024

/* Helper to create pseudo-random values without external dependencies */
static inline double pseudo_rand(int i, int seed) {
    return ((i * 1103515245 + seed) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    /* Use argc to create some variability in the loop */
    int offset = (argc > 1) ? (atoi(argv[1]) % 4) : 2;
    int seed = (argc > 2) ? atoi(argv[2]) : 12345;
    
    /* Declare arrays with different types to create register pressure */
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE];
    double array_d[SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = pseudo_rand(i, seed);
        array_b[i] = pseudo_rand(i + 1000, seed + 1);
        array_c[i] = 0.0;
        array_d[i] = pseudo_rand(i + 2000, seed + 2);
    }
    
    /* Volatile variable to prevent constant propagation */
    volatile int vol_offset = offset;
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Additional variables to create complex dependencies */
    double prev_cond = 0.0;
    double temp_store = 0.0;
    
    /*
     * TARGET LOOP for modulo scheduling
     * This loop contains all the elements needed to trigger the debug output:
     * 1. Loop-carried dependency (reduction pattern)
     * 2. High-latency operations (sqrt, division)
     * 3. Multiple memory accesses with potential aliasing
     * 4. Conditional stores based on previous iteration
     * 5. Non-constant array indices
     */
    for (int i = 0; i < SIZE - 4; i++) {
        /* High-latency operation 1: Square root with dependency */
        double x = sqrt(array_a[i] + 1.0);
        
        /* High-latency operation 2: Division with non-constant divisor */
        double y = array_d[i + vol_offset] / (x + 0.5);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + x * y;
        
        /* Complex memory access with non-constant offset */
        double z = array_b[i + (vol_offset % 3)] * array_a[i + 1];
        
        /* Another high-latency operation */
        double w = sin(z * 0.01) + cos(y * 0.01);
        
        /* Conditional store with loop-carried dependency */
        if (i > 0 && prev_cond > 0.5) {
            /* Store with potential aliasing (no restrict keyword) */
            array_c[i] = x + w;
            temp_store = array_c[i - 1] * 0.8;
        }
        
        /* Update condition for next iteration (distance-1 dependency) */
        prev_cond = array_b[i] * 0.3 + temp_store * 0.7;
        
        /* Additional memory access to create more pressure */
        array_d[i + 2] = array_d[i + 2] * 0.95 + w * 0.05;
        
        /* Another reduction-like operation */
        sum = sum + array_a[i + vol_offset] * 0.1;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %f\n", sum);
    
    /* Additional print to use arrays and prevent optimization */
    double check = 0.0;
    for (int i = 0; i < SIZE; i++) {
        check += array_c[i];
    }
    printf("Check: %f\n", check);
    
    return 0;
}
