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
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Seed based on argv to create variation */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        /* Simple pseudo-random initialization */
        array_a[i] = (double)((seed + i * 17) % 100) / 10.0;
        if (i < SIZE) {
            array_b[i] = (double)((seed + i * 23) % 100) / 10.0 - 5.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create conditional dependency chain */
    double prev_condition = 0.0;
    
    /* Get offset - volatile prevents optimization */
    int k = g_offset;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required patterns:
     * 1. Loop-carried dependency (reduction)
     * 2. High-latency operations (sqrt, division)
     * 3. Multiple memory accesses with potential aliasing
     * 4. Conditional store with cross-iteration dependency
     */
    for (int i = 0; i < SIZE; i++) {
        /* 1. High-latency operation: sqrt with division */
        double x = sqrt(array_a[i + k] + 1.0);
        x = x / (array_b[i] + 2.0);  /* Non-constant divisor */
        
        /* 2. Reduction with loop-carried dependency (distance-1) */
        sum = sum * 0.99 + x;  /* This creates a true dependency chain */
        
        /* 3. Complex address calculation with multiple arrays */
        double temp = array_a[i] * 0.5 + array_b[(i + 1) % SIZE] * 0.3;
        
        /* 4. Conditional store with cross-iteration dependency */
        if (i > 0 && prev_condition > 0.0) {
            /* This creates both data and control dependencies across iterations */
            array_c[i] = x * temp;
        }
        
        /* Update condition for next iteration */
        prev_condition = array_b[i];
        
        /* Additional memory access with offset to create more pressure */
        array_a[i + (k % 4)] = array_a[i + (k % 4)] * 0.9 + 0.1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    
    /* Simple checksum to verify computation */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
