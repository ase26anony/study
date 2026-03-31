#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is crafted to trigger GCC's modulo scheduler debugging output
 * for specific edges in the dependence graph (DDG) during backtracking.
 * The loop contains all necessary elements:
 * 1. Loop-carried dependencies (reduction pattern)
 * 2. High-latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Conditional stores based on previous iteration values
 */

#define SIZE 1024

/* Use volatile to prevent constant propagation and force memory accesses */
static volatile int offset = 3;

int main(int argc, char *argv[]) {
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE];
    double sum = 0.0;
    int i;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < SIZE; i++) {
        array_a[i] = (double)((i * 13 + 7) % 100) / 10.0;
        array_b[i] = (double)((i * 17 + 11) % 100) / 10.0;
        array_c[i] = 0.0;
    }
    
    /* Use command line argument to create variability in offset */
    if (argc > 1) {
        offset = atoi(argv[1]) % 5 + 1;
    }
    
    /*
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required patterns:
     */
    for (i = 0; i < SIZE - offset; i++) {
        /* 1. HIGH LATENCY OPERATION: sqrt with division */
        double x = sqrt(array_a[i] + 1.0) / 1.5;
        
        /* 2. LOOP-CARRIED REDUCTION (distance-1 dependency) */
        /* This creates a true recurrence: sum depends on previous iteration's sum */
        sum = sum * 0.99 + x;
        
        /* 3. MULTIPLE MEMORY ACCESSES with non-constant offsets */
        /* Access array_a with offset to create complex address calculation */
        double y = array_a[i + offset] * 2.0;
        
        /* 4. CONDITIONAL STORE based on previous iteration value */
        /* This creates control and data dependencies across iterations */
        if (i > 0 && array_b[i - 1] > 0.0) {
            array_c[i] = x + y;
        }
        
        /* Additional high-latency operation to increase register pressure */
        array_b[i] = array_a[i] / (sum + 0.001);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Sample array_c[%d]: %f\n", SIZE/2, array_c[SIZE/2]);
    
    return 0;
}
