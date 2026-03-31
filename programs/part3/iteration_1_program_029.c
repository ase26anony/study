#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* Prevent compiler from optimizing away the loop */
volatile int offset = 1;

/* Function to generate pseudo-random values */
double rand_val(int i) {
    return ((i * 1103515245 + 12345) & 0x7fffffff) / 2147483648.0;
}

int main(int argc, char *argv[]) {
    /* Use argc to create some variability in the data */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Declare arrays with no restrict keyword to force aliasing assumptions */
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = rand_val(i + seed) * 100.0 + 1.0;
        array_b[i] = rand_val(i + seed * 2) * 50.0 - 25.0;
        array_c[i] = 0.0;
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create non-constant index calculations */
    int k = offset;
    
    /* The target innermost loop for modulo scheduling */
    for (int i = 0; i < SIZE - 1; i++) {
        /* High latency operation: square root */
        double x = sqrt(array_a[i] + 1.0);
        
        /* Another high latency operation: division with non-constant divisor */
        double y = x / (array_b[i] + 2.0);
        
        /* Reduction with distance-1 dependency (true recurrence) */
        sum = sum * 0.999 + y;
        
        /* Access multiple arrays with non-constant offsets */
        double temp_a = array_a[i + k];      /* Non-constant offset */
        double temp_b = array_b[(i * 3) % SIZE]; /* Complex addressing */
        
        /* High latency operation: trigonometric function */
        double z = sin(temp_a) * cos(temp_b);
        
        /* Conditional store with loop-carried dependency */
        if (i > 0 && array_b[i-1] > 0.0) {
            array_c[i] = sum + z;
        }
        
        /* Additional memory access with potential aliasing */
        array_a[i] = array_a[i] * 0.9 + z * 0.1;
        
        /* Another reduction-like operation */
        array_b[i+1] = array_b[i] * 0.8 + sum * 0.2;
    }
    
    /* Final computation to prevent dead code elimination */
    double result = sum + array_c[SIZE/2];
    
    /* Print result to ensure computation isn't optimized away */
    printf("Result: %f\n", result);
    
    return (int)(result * 1000) % 256;
}
