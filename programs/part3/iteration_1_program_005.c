#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* Function to prevent compiler from optimizing away the loop */
static volatile int force_no_inline = 0;

/* Helper to create pseudo-random values */
static double pseudo_rand(int seed) {
    return ((seed * 1103515245 + 12345) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    double sum = 0.0;
    int i;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < SIZE; i++) {
        array_a[i] = pseudo_rand(i + (argc > 1 ? atoi(argv[1]) : 42));
        array_b[i] = pseudo_rand(i * 3 + (argc > 2 ? atoi(argv[2]) : 17));
    }
    
    /* Create a small offset that varies to prevent constant propagation */
    int offset = (argc > 3) ? (atoi(argv[3]) % 8) : 3;
    if (offset < 1) offset = 1;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop has all the characteristics needed:
     * 1. Innermost loop with carried dependencies
     * 2. High latency operations (sqrt, division)
     * 3. Multiple memory accesses with potential aliasing
     * 4. Loop-carried conditional stores
     */
    for (i = 0; i < SIZE - offset; i++) {
        /* High latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0);
        double y = array_a[i] / (array_b[i] + 0.001);  /* Avoid division by zero */
        
        /* Reduction with carried dependency (distance-1) */
        sum = sum * 0.999 + x * y;  /* This creates a recurrence */
        
        /* Access multiple arrays with non-constant offsets */
        double temp = array_b[i + offset] * 0.5;
        
        /* Loop-carried conditional store */
        if (i > 0) {
            /* Condition depends on previous iteration's value */
            if (array_c[i - 1] > 0.5) {
                array_c[i] = x + temp;
            } else {
                array_c[i] = y - temp;
            }
        } else {
            array_c[i] = x + y;
        }
        
        /* Additional memory access with potential aliasing */
        array_b[i] = array_b[i] * 0.9 + sum * 0.1;
        
        /* Another high-latency operation using sin() */
        if (i % 4 == 0) {
            array_a[i] = sin(array_a[i] * 0.01);
        }
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Array_c[%d]: %f\n", SIZE/2, array_c[SIZE/2]);
    
    /* Force compiler to keep all computations */
    if (force_no_inline) {
        printf("Debug: %f %f\n", array_a[0], array_b[0]);
    }
    
    return (int)(sum * 1000) % 256;
}
