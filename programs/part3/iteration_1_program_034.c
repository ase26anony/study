#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler debug output
 * Specifically targeting lines 596-606 in modulo-sched.cc
 */

#define SIZE 1024

/* Helper to create pseudo-random values without external dependencies */
static inline double pseudo_rand(int i, int seed) {
    return ((i * 1103515245 + 12345 + seed) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    /* Use argc to create some variability in the computation */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Declare arrays with different types to create register pressure */
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    int array_d[SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = pseudo_rand(i, seed);
        array_b[i] = pseudo_rand(i * 2, seed + 1);
        array_d[i] = (int)(pseudo_rand(i * 3, seed + 2) * 1000);
    }
    
    /* Volatile variable to prevent constant propagation */
    volatile int offset = 3;
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable with distance-1 dependency */
    double prev = 0.0;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required patterns:
     */
    for (int i = 0; i < SIZE; i++) {
        /* 1. High-latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0);
        x = x / (array_b[i] + 0.001);  /* Division adds latency */
        
        /* 2. Loop-carried reduction with recurrence */
        sum = sum * 0.999 + x;  /* Distance-1 dependency on sum */
        
        /* 3. Multiple memory accesses with non-constant offsets */
        /* Create complex address calculation */
        int idx1 = (i + offset) % SIZE;
        int idx2 = (i + offset / 2) % SIZE;
        
        /* Memory operations that may alias */
        double temp1 = array_a[idx1];
        double temp2 = array_b[idx2];
        
        /* 4. Conditional store with loop-carried dependency */
        if (i > 0 && array_d[i-1] > 500) {  /* Distance-1 dependency */
            /* High-latency operation in conditional path */
            double y = sin(temp1 * 0.01);  /* Another high-latency op */
            array_c[i] = y + prev;  /* Uses value from previous iteration */
        }
        
        /* 5. Additional recurrence for more distance-1 edges */
        prev = temp1 * 0.5 + temp2 * 0.5;
        
        /* 6. Integer division with variable divisor (high latency) */
        if (array_d[i] != 0) {
            int div_result = 1000 / (array_d[i] + 1);  /* Variable divisor */
            array_d[i] = (array_d[i] + div_result) / 2;
        }
        
        /* 7. More complex memory access pattern */
        /* Access with stride that's not compile-time constant */
        int stride = (i % 4) + 1;
        if (i + stride < SIZE) {
            array_b[i + stride] = array_b[i + stride] * 0.99 + x;
        }
    }
    
    /* Prevent dead code elimination */
    printf("Result: sum = %f, array_c[%d] = %f\n", 
           sum, SIZE-1, array_c[SIZE-1]);
    
    /* Additional checksum to ensure all computations are used */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i] + array_d[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return (int)(sum + checksum) % 100;
}
