#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SIZE 1024

/* Volatile variable to prevent constant propagation */
volatile int offset = 1;

/* Function to create pseudo-random values */
double rand_val(int i) {
    return (double)((i * 1103515245 + 12345) & 0x7fffffff) / 0x7fffffff;
}

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random data */
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    
    /* Use argv to create variation and prevent optimization */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = rand_val(i + seed);
        array_b[i] = rand_val(i + seed * 2);
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = array_a[0];
    
    /* Target loop for modulo scheduling */
    for (int i = 0; i < SIZE - offset; i++) {
        /* High latency operation: square root */
        double x = sqrt(array_a[i] + 1.0);
        
        /* Another high latency operation: division with non-constant divisor */
        double y = array_b[i + offset] / (x + 2.0);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.99 + x * y;
        
        /* Conditional store with loop-carried dependency */
        if (i > 0) {
            /* Condition depends on value from previous iteration */
            if (array_b[i - 1] > 0.5) {
                /* Complex address calculation with offset */
                array_c[i + (offset % 3)] = x + sum * 0.1;
            }
        }
        
        /* Additional memory access with aliasing possibility */
        array_a[i + (offset % 2)] = array_b[i] * 0.8 + sum * 0.01;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %f\n", sum + array_c[SIZE/2]);
    
    return 0;
}
