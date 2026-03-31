#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SIZE 1024

/* Volatile variable to prevent constant propagation */
volatile int offset = 1;

/* Function to generate pseudo-random values */
double rand_val(int i) {
    return ((i * 1103515245 + 12345) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    /* Use argc to create runtime-dependent values */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Declare arrays with potential aliasing (no restrict) */
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = rand_val(i + seed) * 100.0;
        array_b[i] = rand_val(i + seed * 2) * 50.0 - 25.0;
        array_c[i] = 0.0;
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create conditional dependency across iterations */
    double prev_condition = 0.0;
    
    /* The target innermost loop for modulo scheduling */
    for (int i = 0; i < SIZE - 2; i++) {
        /* High latency operation: square root */
        double x = sqrt(array_a[i] + 1.0);
        
        /* Another high latency operation: division with non-constant divisor */
        double y = x / (array_b[i] + 2.5);
        
        /* Reduction with distance-1 dependency (true recurrence) */
        sum = sum * 0.999 + y;
        
        /* Multiple memory accesses with non-constant offsets */
        double temp1 = array_a[i + offset];      /* i+1 access */
        double temp2 = array_b[i + (offset % 2)]; /* i+0 or i+1 */
        
        /* Complex operation mixing values */
        double z = temp1 * 0.3 + temp2 * 0.7;
        
        /* Loop-carried conditional store */
        if (i > 0) {
            /* Condition depends on previous iteration's value */
            if (prev_condition > 0.5) {
                array_c[i] = z + sum * 0.1;
            }
        }
        
        /* Update condition for next iteration (creates dependency) */
        prev_condition = fmod(z, 1.0);
        
        /* Additional high-latency operation: trigonometric function */
        if (i % 3 == 0) {
            array_b[i + 1] += sin(array_a[i] * 0.01);
        }
        
        /* Another memory access pattern */
        array_a[i + 1] = array_a[i + 1] * 0.95 + sum * 0.05;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", sum);
    
    /* Also print a checksum of array_c */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return (int)(sum + checksum) % 100;
}
