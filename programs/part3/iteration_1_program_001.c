#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler debugging output
 * for the specific block in modulo-sched.cc lines 596-606.
 * 
 * The loop contains:
 * 1. Loop-carried dependencies (reduction pattern)
 * 2. High-latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Conditional stores based on previous iteration
 */

#define SIZE 1024
#define OFFSET 3

/* Use volatile to prevent constant propagation and force memory accesses */
static volatile int g_offset = OFFSET;

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Seed based on arguments to make values runtime-dependent */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = (double)((seed + i * 37) % 1000) / 10.0;
        if (i < SIZE) {
            array_b[i] = (double)((seed + i * 73) % 1000) / 10.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Get offset from volatile to prevent constant propagation */
    int k = g_offset;
    
    /*
     * TARGET LOOP for modulo scheduling
     * This loop should trigger the debug output in modulo-sched.cc
     */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation 1: square root with division */
        double x = sqrt(array_a[i + k] + 1.0);
        x = x / (array_b[i] + 0.5);  /* Non-constant divisor */
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + x;  /* sum depends on previous iteration's sum */
        
        /* Conditional store based on previous iteration */
        if (i > 0 && array_b[i-1] > 0.0) {
            /* This creates a memory dependency chain */
            array_c[i] = x * array_c[i-1] * 0.5;
        } else {
            array_c[i] = x;
        }
        
        /* Additional high-latency operation mixing arrays */
        double y = array_a[i] / (array_b[i] + 0.1);  /* Another division */
        sum += y * 0.01;
        
        /* Access with non-constant offset to create complex addressing */
        if (i + 2 < SIZE) {
            array_b[i+1] += array_a[i+2] * 0.1;
        }
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Sample values: %f %f %f\n", array_c[10], array_c[100], array_c[1000]);
    
    return (int)(sum * 100) % 256;
}
