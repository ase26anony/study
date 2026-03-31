#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is crafted to trigger specific debugging output in GCC's modulo scheduler.
 * It creates a loop with the characteristics needed for the scheduler to generate
 * the debug output containing the uncovered lines.
 */

#define SIZE 1024

/* Helper to create pseudo-random values without external dependencies */
static inline double pseudo_rand(int i, int seed) {
    return ((i * 1103515245 + seed) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    /* Use argc to create some variability in the data */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Declare arrays with different types to create register pressure */
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE];
    int array_d[SIZE];
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = pseudo_rand(i, seed);
        array_b[i] = pseudo_rand(i, seed + 1);
        array_c[i] = 0.0;
        array_d[i] = (int)(pseudo_rand(i, seed + 2) * 100);
    }
    
    /* Volatile variable to prevent constant propagation */
    volatile int offset = 3;
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create conditional store dependency */
    double prev_value = 0.0;
    
    /* 
     * TARGET LOOP: This is the loop we want the modulo scheduler to pipeline.
     * It contains all the characteristics needed to trigger the debug output:
     */
    for (int i = 0; i < SIZE - offset; i++) {
        /* 1. High-latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0);
        double y = x / (array_b[i] + 0.001);  /* Division has high latency */
        
        /* 2. Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + y;  /* True distance-1 recurrence */
        
        /* 3. Multiple memory accesses with non-constant offsets */
        /* Creates complex address calculations and potential aliasing */
        double temp1 = array_a[i + (offset % 4)];
        double temp2 = array_b[i + ((offset + 1) % 4)];
        
        /* 4. Conditional store based on previous iteration */
        /* This creates control and data dependencies across iterations */
        if (i > 0 && array_d[i-1] > 50) {
            array_c[i] = prev_value + temp1;
        }
        
        /* 5. Additional high-latency operation: sin() */
        /* Mixes with the reduction to create complex critical path */
        double z = sin(y * 0.01);
        
        /* 6. Another reduction-like operation with carried dependency */
        prev_value = z * 0.5 + prev_value * 0.5;
        
        /* 7. More memory accesses with potential aliasing */
        /* No restrict keyword, so compiler must assume aliasing */
        array_d[i] = (int)((array_a[i] + array_b[i]) * 100) % 100;
        
        /* 8. Integer division (high latency on many architectures) */
        if (array_d[i] != 0) {
            int divisor = array_d[i] | 1;  /* Ensure non-zero */
            array_d[i] = (1000 / divisor) % 100;
        }
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Final prev_value: %f\n", prev_value);
    
    /* Simple checksum to verify computation */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i] + array_d[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
