#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger specific uncovered lines in GCC's modulo scheduler
 * by creating a loop with complex dependencies that forces the scheduler to analyze
 * distance-1 dependencies and high-latency operations.
 */

#define SIZE 1024
#define OFFSET 3

/* Use volatile to prevent constant propagation and force memory accesses */
volatile int g_offset = OFFSET;

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Use argc to create non-constant initialization values */
    double seed = (argc > 1) ? atof(argv[1]) : 3.14159;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = sin(seed * i * 0.01) + 1.0;
        if (i < SIZE) {
            array_b[i] = cos(seed * i * 0.02) - 0.5;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create conditional dependency across iterations */
    double prev_condition = 0.0;
    
    /* Get offset from volatile to prevent constant propagation */
    int offset = g_offset;
    
    /* 
     * TARGET LOOP: This is the loop we want modulo-scheduled
     * It contains all the necessary elements to trigger the uncovered code:
     * 1. Loop-carried dependency (reduction pattern)
     * 2. High-latency operations (division, sqrt)
     * 3. Multiple memory accesses with potential aliasing
     * 4. Conditional store based on previous iteration
     */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation 1: Division with non-constant divisor */
        double temp1 = array_a[i] / (array_b[i] + 2.0);
        
        /* High-latency operation 2: Square root */
        double x = sqrt(temp1 + 1.0);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + x;
        
        /* Access array with non-constant offset (creates complex addressing) */
        double neighbor = array_a[i + offset % 8];
        
        /* Another high-latency operation */
        double y = neighbor / (sum + 0.001);
        
        /* Conditional store with dependency on previous iteration */
        if (i > 0 && prev_condition > 0.0) {
            /* This creates a store that depends on computation from previous iteration */
            array_c[i] = x * y + array_b[i-1];
        }
        
        /* Update condition for next iteration (distance-1 dependency) */
        prev_condition = array_b[i] * 0.5;
        
        /* Additional memory access with potential aliasing */
        array_b[i] = array_b[i] + sin(y) * 0.1;
        
        /* Another reduction-like operation */
        sum = sum + cos(array_a[i]) * 0.01;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: sum = %f, array_c[%d] = %f\n", 
           sum, SIZE-1, array_c[SIZE-1]);
    
    /* Additional computation to ensure all arrays are used */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
