#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is crafted to trigger GCC's modulo scheduler debug output
 * by creating a loop with specific characteristics:
 * 1. Loop-carried dependencies (reduction pattern)
 * 2. High-latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Conditional stores based on previous iteration values
 */

#define SIZE 1024

/* Use volatile to prevent constant propagation and force memory accesses */
static volatile int offset = 3;

int main(int argc, char *argv[]) {
    double array_a[SIZE + 10];  /* Extra space for offset accesses */
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Initialize with pseudo-random values based on argv */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE + 10; i++) {
        /* Simple pseudo-random initialization */
        array_a[i] = (double)((seed + i * 17) % 100) / 10.0 + 1.0;
        if (i < SIZE) {
            array_b[i] = (double)((seed + i * 23) % 100) / 10.0 - 5.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Target loop for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation 1: square root */
        double x = sqrt(array_a[i] + 1.0);
        
        /* High-latency operation 2: division with non-constant divisor */
        double y = x / (array_b[i] + 2.5);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + y;
        
        /* Access multiple arrays with non-constant offset */
        /* The offset is volatile to prevent optimization */
        double z = array_a[i + offset] * 0.5;
        
        /* Conditional store based on previous iteration value */
        if (i > 0 && array_b[i-1] > 0.0) {
            array_c[i] = sum + z;
        }
        
        /* Additional memory access with potential aliasing */
        /* No restrict keyword, so compiler must assume aliasing */
        array_b[i] = array_b[i] * 0.8 + z;
        
        /* Another high-latency operation mixed in */
        if (i % 4 == 0) {
            array_a[i] = array_a[i] / (sum + 0.1);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Sample values: array_c[100]=%f, array_c[500]=%f\n", 
           array_c[100], array_c[500]);
    
    return 0;
}
