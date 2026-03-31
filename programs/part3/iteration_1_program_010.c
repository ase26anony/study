#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* Helper to create pseudo-random values */
static inline double rand_val(int i, int seed) {
    return ((i * 1103515245 + seed) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    double sum = 0.0;
    int i;
    
    /* Use argv for some variability in initialization */
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < SIZE; i++) {
        array_a[i] = rand_val(i, seed);
        array_b[i] = rand_val(i * 3, seed + 1);
    }
    
    /* Volatile variable to prevent constant propagation */
    volatile int offset = 3;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required patterns:
     */
    for (i = 0; i < SIZE; i++) {
        /* 1. High-latency operation: sqrt */
        double x = sqrt(array_a[i] + 1.0);
        
        /* 2. Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.99 + x;  /* True recurrence: sum depends on previous iteration's sum */
        
        /* 3. Conditional store with carried dependency */
        if (i > 0 && array_b[i-1] > 0.5) {  /* Distance-1 memory dependency */
            array_c[i] = x * 2.0;
        }
        
        /* 4. Multiple memory accesses with non-constant offsets */
        /* Access with offset to create complex addressing */
        if (i + offset < SIZE) {
            /* Another high-latency operation: division */
            double y = array_a[i + offset] / (array_b[i] + 0.001);
            sum += y * 0.1;
        }
        
        /* Additional memory access to increase register pressure */
        array_b[i] = array_b[i] * 0.9 + sum * 0.01;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %f\n", sum);
    
    /* Use array_c to prevent elimination */
    double check = 0.0;
    for (i = 0; i < SIZE; i++) {
        check += array_c[i];
    }
    printf("Check: %f\n", check);
    
    return 0;
}
