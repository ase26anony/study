#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler debugging output
 * by creating a loop with specific characteristics:
 * 1. Loop-carried dependencies (reduction pattern)
 * 2. High-latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Conditional stores based on previous iterations
 */

#define SIZE 1024

/* Use volatile to prevent constant propagation and force actual computation */
static volatile int offset = 1;

int main(int argc, char *argv[]) {
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE];
    double sum = 0.0;
    int i;
    
    /* Initialize arrays with pseudo-random values based on argv */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    for (i = 0; i < SIZE; i++) {
        array_a[i] = (double)((seed + i * 37) % 100) / 10.0 + 1.0;
        array_b[i] = (double)((seed + i * 53) % 100) / 10.0 - 5.0;
        array_c[i] = 0.0;
    }
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop has all the characteristics needed to trigger the debug output:
     */
    for (i = 0; i < SIZE - offset; i++) {
        /* 1. High-latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0) / 2.5;
        
        /* 2. Reduction with loop-carried dependency (distance-1) */
        /*    sum depends on previous iteration's sum */
        sum = sum * 0.99 + x;
        
        /* 3. Multiple array accesses with non-constant offsets */
        /*    Creates complex address calculations and potential aliasing */
        double temp = array_b[i + offset] * 1.1;
        
        /* 4. Conditional store based on value from previous iteration */
        /*    Creates control and data dependencies across iterations */
        if (i > 0 && array_b[i-1] > 0.0) {
            array_c[i] = x + temp;
        }
        
        /* Additional high-latency operation to increase register pressure */
        array_a[i] = array_a[i] / (sum + 0.001);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: sum = %f, array_c[%d] = %f\n", 
           sum, SIZE/2, array_c[SIZE/2]);
    
    return (int)(sum * 1000) % 256;
}
