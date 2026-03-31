#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler debug output
 * for specific edge moves during backtracking/scheduling.
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -dP -std=c99
 */

#define SIZE 1024
#define OFFSET 3

/* Use volatile to prevent constant propagation and force memory accesses */
static volatile int g_offset = OFFSET;

int main(int argc, char *argv[]) {
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Initialize with pseudo-random values based on argv */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = (double)((seed + i * 13) % 100) / 10.0 + 1.0;
        if (i < SIZE) {
            array_b[i] = (double)((seed + i * 17) % 100) / 10.0 - 5.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create conditional store dependency */
    double prev_condition = 0.0;
    
    /* Get offset from volatile to prevent constant propagation */
    int k = g_offset;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the necessary elements:
     * 1. High-latency operations (sqrt, division)
     * 2. Loop-carried reduction dependency
     * 3. Conditional store with cross-iteration dependency
     * 4. Multiple array accesses with non-constant offsets
     */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation: sqrt with division */
        double base_val = array_a[i + k] + 1.0;
        double x = sqrt(base_val);
        
        /* Additional high-latency operation: division with non-constant divisor */
        double divisor = (array_b[i] > 0.0) ? array_b[i] : 1.5;
        x = x / divisor;
        
        /* Loop-carried reduction (distance-1 dependency) */
        sum = sum * 0.99 + x;  /* This creates a true recurrence */
        
        /* Conditional store with cross-iteration dependency */
        if (i > 0 && prev_condition > 0.0) {
            /* Store depends on value from previous iteration */
            array_c[i] = x * prev_condition;
        }
        
        /* Update condition for next iteration (creates dependency chain) */
        prev_condition = array_b[i] + sum * 0.1;
        
        /* Access multiple arrays with complex addressing */
        double temp = array_a[i] + array_b[(i + 1) % SIZE];
        if (temp > 10.0) {
            /* Another high-latency operation in conditional path */
            array_a[i + k] = array_a[i + k] / (sum + 1.0);
        }
    }
    
    /* Prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Array_c[100]: %f\n", array_c[100]);
    
    return (int)(sum * 100) % 256;
}
