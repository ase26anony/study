#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent aggressive optimizations */
volatile int offset = 1;

/* Function to create pseudo-random values */
double pseudo_rand(int i) {
    return ((i * 1103515245 + 12345) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random data */
    double array_a[SIZE + 2];
    double array_b[SIZE + 2];
    double array_c[SIZE + 2];
    
    /* Use argv to create variation in initialization */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE + 2; i++) {
        array_a[i] = pseudo_rand(i + seed) * 100.0;
        array_b[i] = pseudo_rand(i + seed * 2) * 50.0 - 25.0;
        array_c[i] = 0.0;
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = array_a[0] * 0.5;
    
    /* Variable to create conditional store dependency */
    double prev_condition = array_b[0];
    
    /* Target innermost loop for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* 1. High-latency operation: division and sqrt */
        double temp = array_a[i] + 1.0;
        double x = sqrt(temp) / (temp + 0.001);  /* Division creates high latency */
        
        /* 2. Update reduction with carried dependency (distance-1) */
        sum = sum * 0.99 + x;  /* True recurrence: sum depends on previous iteration */
        
        /* 3. Conditional store based on previous iteration value */
        if (i > 0 && prev_condition > 0.0) {
            array_c[i] = x * 2.0;  /* Store depends on prev_condition from last iteration */
        }
        
        /* 4. Multiple array accesses with non-constant offsets */
        /* Access with offset to prevent simple addressing */
        double val1 = array_a[i + offset];      /* Non-constant offset */
        double val2 = array_b[i + (offset % 2)]; /* Different offset pattern */
        
        /* 5. Additional high-latency operation with carried dependency */
        prev_condition = (prev_condition + val1) / (val2 + 1.5);  /* Division + recurrence */
        
        /* 6. Another memory access pattern */
        array_b[i + 1] = array_b[i] * 0.9 + val1 * 0.1;  /* Distance-1 store */
    }
    
    /* Use results to prevent dead code elimination */
    printf("Final sum: %f\n", sum);
    printf("Array_c[%d]: %f\n", SIZE/2, array_c[SIZE/2]);
    
    return 0;
}
