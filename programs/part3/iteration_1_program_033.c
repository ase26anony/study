#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler debug output
 * Specifically targeting lines 596-606 in modulo-sched.cc
 */

#define SIZE 1024

/* Use volatile to prevent constant propagation and force actual computation */
static volatile int offset = 3;

int main(int argc, char *argv[]) {
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    double sum = 0.0;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (double)((i * 13 + 7) % 100) / 10.0;
        array_b[i] = (double)((i * 17 + 11) % 100) / 10.0;
    }
    
    /* Use command line argument to create variability */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    offset = (seed % 5) + 1;
    
    /*
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required patterns:
     * 1. Loop-carried dependency through 'sum' (reduction)
     * 2. High-latency operations (sqrt, division)
     * 3. Multiple memory accesses with potential aliasing
     * 4. Conditional store with cross-iteration dependency
     */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation 1: square root */
        double x = sqrt(array_a[i] + 1.0);
        
        /* High-latency operation 2: division with non-constant divisor */
        double divisor = (array_b[i] + 0.5);
        if (divisor != 0.0) {
            x = x / divisor;
        }
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.99 + x;
        
        /* Access multiple arrays with non-constant offset */
        int idx = i + offset;
        if (idx < SIZE) {
            /* Create complex address calculation */
            double temp = array_a[idx] * 0.7 + array_b[i] * 0.3;
            
            /* Conditional store with cross-iteration dependency */
            if (i > 0 && array_b[i-1] > 0.0) {
                array_c[i] = temp + sum * 0.1;
            }
        }
        
        /* Additional memory access to create register pressure */
        if (i > 1) {
            array_a[i-1] = array_a[i-1] * 0.95 + array_c[i-2] * 0.05;
        }
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    
    /* Compute checksum to ensure all computations matter */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
    }
    printf("Array checksum: %f\n", checksum);
    
    return (int)(sum + checksum) % 100;
}
