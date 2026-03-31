#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is crafted to trigger GCC's modulo scheduler debug output
 * for specific edge scheduling moves with distance-1 dependencies.
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -dP -std=c99
 */

#define SIZE 1024
#define OFFSET 3

/* Use volatile to prevent constant propagation */
static volatile int offset_arg = OFFSET;

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Use argc to create non-constant initial values */
    double seed = (argc > 1) ? atof(argv[1]) : 3.14159;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = sin(seed * i) * 100.0;
        if (i < SIZE) {
            array_b[i] = cos(seed * i) * 50.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Get offset from volatile to prevent constant propagation */
    int k = offset_arg;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the necessary patterns:
     * 1. High-latency operations (division, sqrt)
     * 2. Distance-1 recurrence (sum depends on previous iteration)
     * 3. Conditional store with carried dependency
     * 4. Multiple array accesses with non-constant offsets
     */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation 1: Division with non-constant divisor */
        double divisor = array_b[i] + 2.5;
        double x = array_a[i + k] / divisor;  /* Non-constant offset access */
        
        /* High-latency operation 2: Square root */
        double y = sqrt(fabs(x) + 1.0);
        
        /* 
         * Distance-1 recurrence: sum depends on previous iteration's sum
         * This creates edges with distance1_uses = true
         */
        sum = sum * 0.999 + y;
        
        /* 
         * Conditional store with carried dependency
         * The condition depends on array_b[i-1] from previous iteration
         */
        if (i > 0 && array_b[i-1] > 0.0) {
            /* Additional high-latency operation in conditional path */
            array_c[i] = sin(sum) * array_a[i];
        }
        
        /* 
         * Another distance-1 operation to increase pressure
         * array_b[i] depends on array_b[i-1] through this update
         */
        if (i > 0) {
            array_b[i] = array_b[i] + array_b[i-1] * 0.1;
        }
        
        /* Additional high-latency operation to create complex DDG */
        array_a[i + k] = array_a[i + k] / (fabs(sum) + 1.0);
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Checksum: %f\n", array_c[SIZE-1] + array_b[SIZE-1]);
    
    return 0;
}
