#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* Helper function to generate pseudo-random values */
static double simple_rand(int seed) {
    return ((seed * 1103515245u + 12345u) & 0x7fffffffu) / 2147483648.0;
}

int main(int argc, char *argv[]) {
    /* Use argc to create some variability in array access patterns */
    int offset = (argc > 1) ? (atoi(argv[1]) % 8) : 3;
    volatile int vol_offset = offset; /* Prevent constant propagation */
    
    /* Declare and initialize arrays */
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = simple_rand(i);
        array_b[i] = simple_rand(i + 1000);
    }
    
    /* Reduction variable with carried dependency */
    double sum = 0.0;
    
    /* The target innermost loop for modulo scheduling */
    for (int i = 0; i < SIZE - 1; i++) {
        /* 1. High-latency operation: sqrt with division */
        double temp = array_a[i] + 1.0;
        double x = sqrt(temp);
        
        /* Mix in another high-latency operation */
        if (temp > 0.5) {
            x = x / (temp + 0.001); /* Non-constant divisor */
        }
        
        /* 2. Reduction with carried dependency (distance-1) */
        sum = sum * 0.99 + x; /* First-order recurrence */
        
        /* 3. Conditional store based on previous iteration */
        if (i > 0 && array_b[i - 1] > 0.5) {
            array_c[i] = x * sum; /* Depends on previous iteration's sum */
        }
        
        /* 4. Multiple array accesses with non-constant offsets */
        int idx = i + vol_offset;
        if (idx < SIZE) {
            /* Create complex address calculation */
            double y = array_a[idx] * array_b[i];
            
            /* Another carried dependency */
            array_b[i + 1] = array_b[i] * 0.9 + y * 0.1;
        }
        
        /* Additional memory access to increase pressure */
        array_a[i] = array_a[i] * 0.95 + sum * 0.05;
    }
    
    /* Final computation to prevent dead code elimination */
    double final_result = sum + array_c[SIZE/2];
    printf("Result: %f\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
