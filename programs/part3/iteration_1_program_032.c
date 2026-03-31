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
#define OFFSET 3

/* Use volatile to prevent constant propagation and force memory accesses */
static volatile int g_offset = OFFSET;

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Seed based on argv to get varying values */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = (double)((seed + i * 37) % 100) / 10.0 + 1.0;
        if (i < SIZE) {
            array_b[i] = (double)((seed + i * 53) % 200) / 10.0 - 10.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create conditional dependency chain */
    double prev_condition = 0.0;
    
    /* Get offset value - volatile prevents optimization */
    int offset = g_offset;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop has all the characteristics needed to trigger the debug output:
     */
    for (int i = 0; i < SIZE; i++) {
        /* 1. High-latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0);
        x = x / (array_a[i + offset] + 0.5);  /* Non-constant divisor */
        
        /* 2. Reduction with loop-carried dependency (distance-1) */
        sum = sum * 0.99 + x;  /* True recurrence: sum depends on previous iteration's sum */
        
        /* 3. Multiple memory accesses with non-constant offsets */
        double temp = array_b[i] * 2.0;
        if (i + 1 < SIZE) {
            temp += array_a[i + 1];  /* Access with offset */
        }
        
        /* 4. Conditional store based on previous iteration */
        if (i > 0 && prev_condition > 0.0) {
            /* This creates a control dependency chain across iterations */
            array_c[i] = x * array_b[i-1];  /* Uses value from previous iteration */
        } else {
            array_c[i] = x;
        }
        
        /* Update condition for next iteration */
        prev_condition = array_b[i] + sum * 0.1;
        
        /* Additional memory access with offset to create more pressure */
        if (i + offset < SIZE) {
            array_b[i] = array_b[i] + array_a[i + offset] * 0.01;
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Array_c[%d]: %f\n", SIZE/2, array_c[SIZE/2]);
    
    return 0;
}
