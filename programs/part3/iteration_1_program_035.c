#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* Helper function to generate pseudo-random values */
static double simple_rand(int seed) {
    return ((seed * 1103515245 + 12345) & 0x7fffffff) / 2147483648.0;
}

int main(int argc, char *argv[]) {
    /* Create arrays with potential aliasing (no restrict keyword) */
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Initialize with pseudo-random values */
    int init_seed = (argc > 1) ? atoi(argv[1]) : 42;
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = simple_rand(init_seed + i);
        array_b[i] = simple_rand(init_seed + i + SIZE);
        array_c[i] = 0.0;
    }
    
    /* Volatile variable to prevent constant propagation */
    volatile int offset = 3;
    
    /* Reduction variable with loop-carried dependency */
    double sum = 0.0;
    
    /* Variable with cross-iteration dependency for conditional */
    double prev_condition = 0.0;
    
    /* The target innermost loop for modulo scheduling */
    for (int i = 0; i < SIZE - offset; i++) {
        /* High-latency operation: square root */
        double x = sqrt(array_a[i] + 1.0);
        
        /* Another high-latency operation: division with non-constant divisor */
        double y = x / (array_b[i] + 0.001);
        
        /* Reduction with distance-1 dependency (true recurrence) */
        sum = sum * 0.999 + y;
        
        /* Access with non-constant offset (creates complex addressing) */
        double neighbor = array_a[i + offset] + array_b[i + (offset - 1)];
        
        /* Loop-carried conditional store */
        if (i > 0 && prev_condition > 0.5) {
            array_c[i] = x * neighbor;
        }
        
        /* Update condition for next iteration (distance-1 dependency) */
        prev_condition = y;
        
        /* Additional memory access with potential aliasing */
        array_b[i] = array_b[i] * 0.9 + sum * 0.1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Sample output: %f\n", array_c[SIZE/2]);
    
    return 0;
}
