#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is crafted to trigger GCC's modulo scheduler debug output
 * for specific edge moves during backtracking/scheduling.
 * The loop contains all necessary patterns to create distance-1 dependencies,
 * high-latency operations, and complex memory access patterns.
 */

#define SIZE 1024

/* Helper to prevent compiler from optimizing away computations */
static volatile int force_no_optimize = 0;

int main(int argc, char *argv[]) {
    /* Use command-line argument to create variability in array access patterns */
    int offset = (argc > 1) ? atoi(argv[1]) % 8 : 3;
    if (offset == 0) offset = 2;  /* Ensure non-zero offset */
    
    /* Declare and initialize arrays with pseudo-random values */
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    
    /* Initialize with simple pseudo-random pattern */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (i * 17 + 23) % 100 / 10.0;
        array_b[i] = (i * 13 + 37) % 100 / 10.0 - 5.0;  /* Some negative values */
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create conditional store dependency */
    double prev_condition = 0.0;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the patterns needed to trigger the debug output:
     * 1. High-latency operations (sqrt, division)
     * 2. Distance-1 reduction dependency
     * 3. Conditional store with cross-iteration dependency
     * 4. Multiple array accesses with non-constant offsets
     */
    for (int i = 0; i < SIZE; i++) {
        /* 
         * HIGH LATENCY OPERATION: sqrt with division
         * This creates nodes with significant latency in the DDG
         */
        double x;
        if (array_a[i] > 0) {
            /* Variable divisor prevents constant folding */
            x = sqrt(array_a[i] + 1.0) / (offset + 1.0);
        } else {
            /* Alternative path to maintain control flow complexity */
            x = sqrt(-array_a[i] + 1.0) / (offset + 2.0);
        }
        
        /* 
         * DISTANCE-1 REDUCTION DEPENDENCY
         * sum depends on its previous iteration value
         * This creates true distance-1 edges in the DDG
         */
        sum = sum * 0.999 + x;
        
        /* 
         * COMPLEX MEMORY ACCESS with non-constant offset
         * Accesses array_a at i + offset (wrapped)
         * The modulo prevents out-of-bounds while maintaining non-constant index
         */
        int idx = (i + offset) % SIZE;
        double temp = array_a[idx] * 0.5;
        
        /* 
         * CONDITIONAL STORE with cross-iteration dependency
         * The condition depends on value from previous iteration
         * This creates additional scheduling constraints
         */
        if (i > 0) {
            /* Dependency on array_b[i-1] creates distance-1 memory dependency */
            if (array_b[i-1] > prev_condition) {
                array_c[i] = x + temp;
                prev_condition = array_c[i];
            } else {
                /* Alternative store to maintain both paths */
                array_c[i] = x - temp;
                prev_condition = -array_c[i];
            }
        } else {
            /* First iteration special case */
            array_c[i] = x;
            prev_condition = x;
        }
        
        /* 
         * ADDITIONAL MEMORY ACCESS with address calculation
         * Creates more register pressure and complex addressing
         */
        array_b[i] = array_b[i] + sum * 0.01;
    }
    
    /* 
     * Use the results to prevent dead code elimination
     * Mix in the volatile variable to force computation
     */
    double result = sum;
    for (int i = 0; i < SIZE; i++) {
        result += array_c[i] * 0.001;
    }
    
    /* Add some noise from command line */
    result += (argc > 1) ? atof(argv[1]) * 0.000001 : 0.0;
    
    /* Force use of volatile to prevent optimization */
    if (force_no_optimize) {
        printf("Dummy: %d\n", force_no_optimize);
    }
    
    printf("Result: %f\n", result);
    return (int)(result * 1000) % 1000;
}
