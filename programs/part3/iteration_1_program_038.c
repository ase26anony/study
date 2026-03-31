#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler debug output
 * for specific edge moves during backtracking/scheduling.
 * The loop contains all necessary patterns to create distance-1 dependencies,
 * high-latency operations, and complex memory access patterns.
 */

#define SIZE 1024
#define OFFSET 3

/* Helper to prevent aggressive optimization */
static volatile int g_offset = OFFSET;

int main(int argc, char *argv[]) {
    /* Use argc to create pseudo-random but deterministic values */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Declare arrays with different types to avoid type-based aliasing assumptions */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    double array_d[SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = (double)((seed + i * 13) % 100) * 0.1;
        if (i < SIZE) {
            array_b[i] = (double)((seed + i * 17) % 100) * 0.2;
            array_c[i] = (double)((seed + i * 19) % 100) * 0.3;
            array_d[i] = (double)((seed + i * 23) % 100) * 0.4;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = array_a[0] * 0.5;
    
    /* Variable to create conditional store dependency */
    double prev_condition = array_b[0];
    
    /* Get offset from volatile to prevent constant propagation */
    int offset = g_offset;
    
    /*
     * TARGET LOOP for modulo scheduling
     * This loop contains all required patterns:
     * 1. High-latency operations (sqrt, division)
     * 2. Distance-1 reduction dependency
     * 3. Conditional store with carried dependency
     * 4. Multiple array accesses with non-constant offsets
     */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation 1: sqrt with dependency on array access */
        double x = sqrt(array_a[i + offset] + 1.0);
        
        /* High-latency operation 2: division with non-constant divisor */
        double y = x / (array_b[i] + 0.001);
        
        /* Loop-carried reduction (distance-1 dependency) */
        sum = sum * 0.99 + y;
        
        /* Access multiple arrays with complex addressing */
        double temp1 = array_c[i] * array_d[(i + 1) % SIZE];
        double temp2 = array_a[(i + offset - 1) % (SIZE + OFFSET)];
        
        /* Conditional store with carried dependency from previous iteration */
        if (i > 0 && prev_condition > 0.0) {
            array_c[i] = sum * temp1;
        }
        
        /* Update condition for next iteration (distance-1 dependency) */
        prev_condition = array_b[i] * 0.5 + temp2;
        
        /* Additional high-latency operation to increase register pressure */
        array_d[i] = sin(array_a[i + offset] * 0.01);
    }
    
    /* Use results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Array checksum: %f\n", array_c[SIZE-1] + array_d[SIZE/2]);
    
    return (int)(sum * 1000) % 256;
}
