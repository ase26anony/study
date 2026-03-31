#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger specific uncovered lines in GCC's modulo scheduler.
 * It creates a loop with the necessary characteristics for modulo scheduling:
 * 1. Loop-carried dependencies (recurrence)
 * 2. High-latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Conditional stores based on previous iterations
 */

#define SIZE 1024
#define OFFSET 3

/* Use volatile to prevent constant propagation and force memory accesses */
static volatile int offset = OFFSET;

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Use argc to create non-constant initialization */
    double seed = (argc > 1) ? atof(argv[1]) : 1.2345;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = sin(seed * i) * 100.0;
        if (i < SIZE) {
            array_b[i] = cos(seed * i) * 50.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 0.0;
    
    /* Target loop for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation 1: sqrt with division */
        double x = sqrt(fabs(array_a[i]) + 1.0);
        
        /* High-latency operation 2: division with non-constant divisor */
        double divisor = (array_b[i] > 0.0) ? array_b[i] : 1.0;
        x = x / (divisor + 0.001);
        
        /* Loop-carried reduction (distance-1 dependency) */
        sum = sum * 0.999 + x;  /* This creates a recurrence */
        
        /* Conditional store based on previous iteration */
        if (i > 0 && array_b[i-1] > 0.0) {
            /* Complex address calculation with non-constant offset */
            int idx = i + (offset % 4);
            if (idx < SIZE) {
                array_c[idx] = x * sum;  /* Uses both x and sum */
            }
        }
        
        /* Access multiple arrays with non-constant offsets */
        /* This creates register pressure and complex address calculations */
        double temp = array_a[i + (offset % 3)] * 0.5;
        if (i < SIZE - 1) {
            array_b[i+1] += temp * 0.1;  /* Another distance-1 dependency */
        }
        
        /* Additional high-latency operation to increase critical path */
        if (i % 8 == 0) {
            sum += sin(array_a[i] * 0.01);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: sum = %f, array_c[100] = %f\n", sum, array_c[100]);
    
    /* Additional computation to ensure loop isn't optimized away */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
