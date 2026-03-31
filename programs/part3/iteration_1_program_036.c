#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler debugging output
 * by creating a loop with specific characteristics:
 * 1. Loop-carried dependencies (distance-1 edges in DDG)
 * 2. High-latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Conditional stores based on previous iterations
 */

#define SIZE 1024

/* Use volatile to prevent constant propagation and force memory accesses */
static volatile int offset = 3;

int main(int argc, char *argv[]) {
    double array_a[SIZE + 10];  /* Extra space for offset accesses */
    double array_b[SIZE + 10];
    double array_c[SIZE + 10];
    
    /* Initialize with pseudo-random values based on argv */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    for (int i = 0; i < SIZE + 10; i++) {
        array_a[i] = (seed + i * 3.14159) / 7.0;
        array_b[i] = (seed + i * 2.71828) / 11.0;
        array_c[i] = 0.0;
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Loop designed for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0) / (array_b[i] + 0.001);
        
        /* Loop-carried reduction (distance-1 dependency) */
        sum = sum * 0.999 + x;  /* This creates a true dependency between iterations */
        
        /* Conditional store based on previous iteration */
        if (i > 0 && array_b[i-1] > 0.5) {
            array_c[i] = x * sum;  /* Uses sum from current iteration */
        }
        
        /* Multiple array accesses with non-constant offset */
        /* No restrict keyword - compiler must assume aliasing */
        double temp = array_a[i + offset] * array_b[i];
        
        /* Another high-latency operation to increase register pressure */
        array_b[i] = temp / (sum + 0.0001);
        
        /* Additional memory access with offset to create complex addressing */
        if (i < SIZE - offset) {
            array_a[i + offset] = array_a[i + offset] * 0.99 + sum;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", sum);
    
    /* Also print a checksum of array_c */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
