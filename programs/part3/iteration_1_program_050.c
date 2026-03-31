#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler logic
 * specifically targeting the debug output block in modulo-sched.cc
 * lines 596-606.
 */

#define SIZE 1024
#define OFFSET 3

/* Helper to prevent compiler from optimizing away computations */
static volatile int force_no_inline = 0;

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Use argc to create some variability in initial values */
    double seed = (argc > 1) ? atof(argv[1]) : 42.0;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = sin(seed + i * 0.1) * 100.0;
        if (i < SIZE) {
            array_b[i] = cos(seed + i * 0.07) * 50.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Variables to create loop-carried dependencies */
    double sum = 1.0;
    double prev_conditional = 0.0;
    
    /* Non-constant offset to force complex address calculations */
    int k = (argc > 2) ? (atoi(argv[2]) % 4) : OFFSET;
    if (k < 1) k = 1;
    
    /*
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required elements:
     * 1. High-latency operations (sqrt, division)
     * 2. Loop-carried reduction dependency
     * 3. Conditional store with cross-iteration dependency
     * 4. Multiple array accesses with non-constant offsets
     */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation 1: sqrt with dependency on array access */
        double x = sqrt(fabs(array_a[i + k]) + 1.0);
        
        /* High-latency operation 2: division with non-constant divisor */
        double y = x / (array_b[i] + 2.0);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + y * 0.001;
        
        /* Conditional store with cross-iteration dependency */
        if (i > 0) {
            /* Dependency on value from previous iteration */
            if (array_b[i-1] > prev_conditional) {
                array_c[i] = x + sum;
                prev_conditional = array_c[i];
            } else {
                array_c[i] = y - sum;
                prev_conditional = array_c[i] * 0.5;
            }
        } else {
            /* First iteration special case */
            array_c[i] = x;
            prev_conditional = x;
        }
        
        /* Additional high-latency operation to increase register pressure */
        double z = sin(array_a[i] * 0.01);
        sum += z * 0.0001;
        
        /* Another memory access with offset to create more dependencies */
        if (i + 2 < SIZE) {
            array_b[i+2] += array_a[i] * 0.01;
        }
    }
    
    /* Prevent dead code elimination */
    if (force_no_inline) {
        printf("Dummy: %p %p %p\n", array_a, array_b, array_c);
    }
    
    /* Print checksum to ensure computations aren't optimized away */
    printf("Final sum: %.10f\n", sum);
    printf("Final array_c[%d]: %.10f\n", SIZE-1, array_c[SIZE-1]);
    
    return 0;
}
