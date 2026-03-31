#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent compiler from optimizing away the loop */
volatile int offset = 1;

/* Function to create pseudo-random values */
double rand_val(int i, int seed) {
    return ((i * 1103515245 + seed) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    double array_a[SIZE + 2];
    double array_b[SIZE + 2];
    double array_c[SIZE + 2];
    double sum = 0.0;
    int i;
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < SIZE + 2; i++) {
        array_a[i] = rand_val(i, seed);
        array_b[i] = rand_val(i, seed * 2);
        array_c[i] = 0.0;
    }
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop has all the characteristics needed:
     * 1. Innermost loop with carried dependencies
     * 2. High latency operations (sqrt, division)
     * 3. Multiple memory accesses with potential aliasing
     * 4. Loop-carried conditional stores
     */
    for (i = 0; i < SIZE; i++) {
        /* High latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0);
        double y = x / (array_b[i] + 0.001);  /* Non-constant divisor */
        
        /* Reduction with carried dependency (distance-1) */
        sum = sum * 0.99 + y;  /* This creates a recurrence */
        
        /* Conditional store based on previous iteration */
        if (i > 0 && array_b[i-1] > 0.5) {
            array_c[i] = x * y;  /* Loop-carried conditional store */
        }
        
        /* Multiple array accesses with non-constant offset */
        /* The volatile offset prevents constant propagation */
        double temp = array_a[i + offset] + array_b[i + (offset % 3)];
        
        /* Another high-latency operation using the temp value */
        if (temp > 0.0) {
            sum += sin(temp) * 0.1;  /* Additional high-latency op */
        }
        
        /* Additional carried dependency through array_b */
        array_b[i+1] = array_b[i] * 0.9 + array_a[i] * 0.1;
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Result: sum = %f, array_c[%d] = %f\n", 
           sum, SIZE/2, array_c[SIZE/2]);
    
    return (int)(sum * 1000) % 256;
}
