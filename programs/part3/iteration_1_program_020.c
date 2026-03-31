#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger specific debugging output in GCC's modulo scheduler.
 * The loop contains all the necessary patterns to expose the uncovered lines:
 * 1. Loop-carried dependencies (reduction with distance-1 edges)
 * 2. High-latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Conditional stores based on previous iteration values
 */

#define SIZE 1024
#define OFFSET 3

/* Use volatile to prevent constant propagation and force memory accesses */
static volatile int g_offset = OFFSET;

int main(int argc, char *argv[]) {
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    double sum = 0.0;
    
    /* Initialize with pseudo-random values based on argv */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = (double)((seed + i * 17) % 100) / 10.0 + 1.0;
        if (i < SIZE) {
            array_b[i] = (double)((seed + i * 23) % 100) / 10.0 - 5.0;
            array_c[i] = 0.0;
        }
    }
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the patterns needed to trigger the uncovered debug output:
     */
    for (int i = 0; i < SIZE; i++) {
        /* 1. High-latency operation with memory access */
        double x = sqrt(array_a[i] + 1.0);
        
        /* 2. Loop-carried reduction (distance-1 dependency) */
        /* This creates edges with distance1_uses = true */
        sum = sum * 0.999 + x / (array_b[i] + 2.0);  /* Division adds latency */
        
        /* 3. Conditional store based on previous iteration */
        /* Creates control and data dependencies across iterations */
        if (i > 0 && array_b[i-1] > 0.0) {
            array_c[i] = x * sum;  /* Uses sum from current iteration */
        }
        
        /* 4. Multiple memory accesses with non-constant offset */
        /* Forces alias analysis and creates more DDG nodes */
        double temp = array_a[i + g_offset] * 0.5;
        array_b[i] = array_b[i] + temp;
        
        /* Additional high-latency operation to increase register pressure */
        if (i % 4 == 0) {
            array_a[i] = array_a[i] / (x + 0.001);  /* Another division */
        }
    }
    
    /* Prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Sample values: array_c[100] = %f, array_c[500] = %f\n", 
           array_c[100], array_c[500]);
    
    return (int)(sum * 1000) % 256;
}
