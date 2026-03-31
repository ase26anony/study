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
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Initialize with pseudo-random values based on argv */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE + 10; i++) {
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
     * This loop contains all the characteristics needed to trigger
     * the specific debugging output in modulo-sched.cc
     */
    for (int i = 0; i < SIZE; i++) {
        /* 
         * 1. HIGH LATENCY OPERATION: sqrt() with division
         * Creates nodes with significant latency in the DDG
         */
        double x = sqrt(array_a[i] + 1.0);
        x = x / (array_a[i + offset] + 0.001);  /* Non-constant divisor */
        
        /* 
         * 2. LOOP-CARRIED REDUCTION (distance-1 dependency)
         * Creates true distance1_uses edges in the DDG
         */
        sum = sum * 0.999 + x;  /* True recurrence: sum depends on previous sum */
        
        /* 
         * 3. CONDITIONAL STORE based on previous iteration
         * Creates control dependencies and additional scheduling constraints
         */
        if (i > 0 && array_b[i - 1] > prev_condition) {
            /* 
             * This store depends on array_b[i-1] from previous iteration
             * and prev_condition which is updated below
             */
            array_c[i] = x * sum;
        }
        
        /* 
         * 4. MULTIPLE MEMORY ACCESSES with non-constant offsets
         * Creates register pressure and complex address calculations
         * No 'restrict' keyword, so compiler must assume aliasing
         */
        double temp = array_a[i + (offset % 5)] * array_b[i];
        
        /* Update condition variable for next iteration */
        prev_condition = temp * 0.5;
        
        /* Additional high-latency operation to increase pressure */
        if (i % 8 == 0) {
            array_b[i] = sin(temp);  /* Another high-latency operation */
        }
    }
    
    /* 
     * 5. SECOND LOOP with different pattern to increase scheduling complexity
     * This creates additional pressure and might cause backtracking
     */
    double acc = 0.0;
    for (int i = 1; i < SIZE; i++) {
        /* First-order recurrence: a[i] depends on a[i-1] */
        array_a[i] = array_b[i] + array_a[i-1] * 0.9;
        
        /* Another reduction with carried dependency */
        acc += array_c[i] / (array_a[i] + 1.0);  /* Division - high latency */
        
        /* Conditional with cross-iteration dependency */
        if (array_a[i-1] > 0.5) {
            array_b[i] = acc * 0.8;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    double result = sum + acc + array_c[SIZE-1];
    
    /* Print result to ensure computations aren't optimized away */
    printf("Result: %f\n", result);
    
    return (int)(result * 1000) % 256;
}
