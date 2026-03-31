#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger specific debugging output in GCC's modulo scheduler.
 * The loop contains:
 * 1. Loop-carried dependencies (reduction pattern)
 * 2. High-latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Conditional stores based on previous iteration values
 */

#define SIZE 1024
#define OFFSET 3

/* Use volatile to prevent constant propagation and force actual computation */
static volatile int g_offset = OFFSET;

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Use argc to create non-constant initialization */
    double seed = (argc > 1) ? atof(argv[1]) : 3.14159;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = sin(seed + i * 0.1) * 100.0;
        if (i < SIZE) {
            array_b[i] = cos(seed + i * 0.07) * 50.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Get offset from volatile to prevent constant propagation */
    int offset = g_offset;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop should trigger the debug output in modulo-sched.cc lines 596-606
     */
    for (int i = 0; i < SIZE; i++) {
        /* 1. High-latency operation: sqrt with division */
        double x = sqrt(fabs(array_a[i] + 1.0) / 2.0);
        
        /* 2. Loop-carried reduction dependency (distance-1) */
        /* Complex reduction with high-latency division */
        sum = sum * 0.999 + x / (fabs(sum) + 1.0);
        
        /* 3. Multiple memory accesses with non-constant offsets */
        /* Access array_a with offset to create complex addressing */
        double y = array_a[i + offset] * 0.5;
        
        /* 4. Conditional store based on previous iteration value */
        /* This creates control and data dependencies across iterations */
        if (i > 0) {
            /* Dependency on array_b[i-1] from previous iteration */
            if (array_b[i-1] > 0.0) {
                /* Store with potential aliasing (no restrict keyword) */
                array_c[i] = x + y + sum;
            }
        }
        
        /* Additional high-latency operation to increase register pressure */
        array_b[i] = array_b[i] / (x + 2.0) + sin(sum * 0.01);
        
        /* Another memory access with offset to create more scheduling complexity */
        if (i + 2 < SIZE) {
            array_a[i + 2] = array_a[i + 2] * 0.99 + sum * 0.01;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Sample values: array_c[100] = %f, array_c[500] = %f\n", 
           array_c[100], array_c[500]);
    
    return (int)(sum * 1000) % 256;
}
