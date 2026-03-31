#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler debug output
 * specifically targeting lines 596-606 in modulo-sched.cc
 * 
 * Key features:
 * 1. Innermost loop with carried dependencies (reduction pattern)
 * 2. High latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Loop-carried conditional stores
 * 5. Non-constant array offsets
 */

#define SIZE 1024
#define OFFSET 3

/* Use volatile to prevent constant propagation */
volatile int g_offset = OFFSET;

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE + OFFSET];
    double array_c[SIZE + OFFSET];
    double sum = 0.0;
    
    /* Seed based on command line argument for variability */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = (double)((seed + i * 1103515245) % 1000) / 100.0 + 1.0;
        array_b[i] = (double)((seed + i * 1664525 + 12345) % 1000) / 100.0;
        array_c[i] = 0.0;
    }
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the necessary characteristics:
     */
    for (int i = 0; i < SIZE; i++) {
        /* 1. High latency operation - sqrt with division */
        double x = sqrt(array_a[i + g_offset] + 1.0);
        x = x / (array_b[i] + 0.001);  /* Division prevents strength reduction */
        
        /* 2. Reduction with carried dependency (distance-1) */
        /* This creates a true loop-carried dependency */
        sum = sum * 0.999 + x;  /* sum depends on previous iteration's sum */
        
        /* 3. Multiple memory accesses with non-constant offsets */
        /* Access different arrays with offset calculations */
        double temp = array_a[i] + array_b[i + 1] - array_a[i + 2];
        
        /* 4. Loop-carried conditional store */
        /* Condition depends on value from previous iteration */
        if (i > 0) {
            /* This creates a memory-based loop-carried dependency */
            if (array_c[i - 1] > 0.5) {  /* Distance-1 memory dependency */
                array_c[i] = x * temp;
            } else {
                array_c[i] = -x * temp;
            }
        } else {
            array_c[i] = x * temp;
        }
        
        /* Additional high-latency operation to increase register pressure */
        array_b[i] = sin(array_a[i] * 0.01) + cos(sum * 0.001);
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    
    /* Compute checksum to ensure all computations are used */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return (int)(sum + checksum) % 100;
}
