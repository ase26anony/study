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

/* Use volatile to prevent constant propagation */
static volatile int g_offset = OFFSET;

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Use argc to create non-constant initialization */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = (double)((seed + i * 37) % 100) / 10.0;
        if (i < SIZE) {
            array_b[i] = (double)((seed + i * 73) % 200) / 20.0 - 5.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create conditional dependency chain */
    double prev_condition = 0.0;
    
    /* Target innermost loop for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* 1. High-latency operation: sqrt with division */
        double base_val = array_a[i + g_offset] + 1.0;
        double x;
        
        /* Mix in a division (high latency) */
        if (base_val > 0.0) {
            x = sqrt(base_val) / 1.234567;
        } else {
            x = sqrt(-base_val + 2.0) / 1.234567;
        }
        
        /* 2. Reduction with carried dependency (distance-1) */
        /* This creates a recurrence: sum depends on previous iteration's sum */
        sum = sum * 0.999 + x * 0.001;
        
        /* 3. Conditional store with loop-carried dependency */
        /* The condition depends on value from previous iteration */
        if (i > 0 && prev_condition > 0.0) {
            array_c[i] = x + array_b[i-1] * 0.5;
        } else {
            array_c[i] = x;
        }
        
        /* Update condition for next iteration (creates distance-1 dependency) */
        prev_condition = array_b[i] + sum * 0.1;
        
        /* 4. Additional memory access with non-constant offset */
        /* Access array_a at different offset to create more memory dependencies */
        double temp = array_a[(i + g_offset/2) % (SIZE + OFFSET)];
        
        /* Use temp in computation to prevent dead code elimination */
        sum += temp * 0.0001;
        
        /* Another high-latency operation: sine calculation */
        if (i % 4 == 0) {
            double angle = sum * 0.01;
            /* Call to libm function - high latency */
            array_b[i] += sin(angle) * 0.1;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Array_c[%d] = %f\n", SIZE-1, array_c[SIZE-1]);
    
    return 0;
}
