/* Test 1: Floating-point intensive computation with nested loops */
#include <math.h>

#define SIZE 256
volatile int g_volatile = 0; /* Prevent optimization */

double __attribute__((noinline)) 
compute_fp_intensive(int iterations) {
    double array[SIZE];
    double result = 0.0;
    int i, j, k;
    
    /* Initialize with some values */
    for (i = 0; i < SIZE; i++) {
        array[i] = sin(i * 0.1) + cos(i * 0.05);
    }
    
    /* Nested loops with mixed operations - creates scheduling pressure */
    for (k = 0; k < iterations; k++) {
        for (i = 1; i < SIZE - 1; i++) {
            for (j = 1; j < SIZE - 1; j++) {
                /* Complex FP operations with dependencies */
                double temp = array[i] * array[j] + 
                             sqrt(fabs(array[i] - array[j])) * 
                             (array[i] / (array[j] + 1.0));
                
                /* Conditional creates branch scheduling needs */
                if (temp > 0.5) {
                    result += temp * log(fabs(temp) + 1.0);
                } else {
                    result -= exp(temp) * 0.5;
                }
                
                /* Memory access pattern */
                array[(i + j) % SIZE] = result * 0.01;
            }
        }
        
        /* Loop-carried dependency */
        array[k % SIZE] = result * 0.1 + g_volatile;
    }
    
    return result;
}
