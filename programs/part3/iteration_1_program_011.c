#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler debug output
 * by creating a loop with specific characteristics:
 * 1. Loop-carried dependencies (reduction pattern)
 * 2. High-latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Conditional stores based on previous iteration values
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
    
    /* Seed based on command line argument to make values runtime-dependent */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        /* Simple pseudo-random initialization */
        array_a[i] = (double)((seed + i * 1103515245) % 1000) / 100.0;
        if (i < SIZE) {
            array_b[i] = (double)((seed + i * 1664525 + 12345) % 1000) / 200.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Additional variable for conditional logic */
    double prev_condition = 0.0;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop has all the characteristics needed to trigger the debug output:
     */
    for (int i = 0; i < SIZE; i++) {
        /* 1. High-latency operation: sqrt with division */
        double x = sqrt(array_a[i + offset] + 1.0);
        x = x / (array_b[i] + 0.001);  /* Non-constant divisor */
        
        /* 2. Reduction with carried dependency (distance-1) */
        sum = sum * 0.999 + x;  /* sum depends on previous iteration's sum */
        
        /* 3. Multiple memory accesses with non-constant offsets */
        double temp = array_a[i] + array_b[(i + 1) % SIZE];
        
        /* 4. Conditional store based on previous iteration */
        if (i > 0 && prev_condition > 0.5) {
            array_c[i] = x * temp;  /* Store depends on previous iteration's condition */
        }
        
        /* Update condition for next iteration (creates another carried dependency) */
        prev_condition = array_b[i] * 0.5 + prev_condition * 0.5;
        
        /* Additional high-latency operation to increase register pressure */
        if (i % 4 == 0) {
            array_b[i] = sin(array_a[i + offset % 4] * 0.01);
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: sum = %f, array_c[%d] = %f\n", 
           sum, SIZE-1, array_c[SIZE-1]);
    
    /* Additional computation to ensure loop isn't optimized away */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return (int)(sum + checksum) % 100;
}
