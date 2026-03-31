#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define OFFSET 3

/* Prevent constant propagation and loop unrolling */
volatile int g_offset = OFFSET;

/* Function to create pseudo-random values */
double pseudo_rand(int i) {
    return (double)((i * 1103515245 + 12345) & 0x7FFFFFFF) / 1073741824.0;
}

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Initialize with values derived from argv to prevent compile-time optimization */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = pseudo_rand(i + seed);
        if (i < SIZE) {
            array_b[i] = pseudo_rand(i * 2 + seed);
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = array_a[0];
    
    /* Get offset from volatile to prevent constant propagation */
    int offset = g_offset;
    
    /* Target loop for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* i. High-latency operation: sqrt with division */
        double temp = array_a[i] + 1.0;
        double x;
        
        /* Use division to create high latency operation */
        if (temp > 0.0) {
            x = sqrt(temp) / 1.234567;  /* Non-constant divisor */
        } else {
            x = 0.0;
        }
        
        /* ii. Update reduction with carried dependency (distance-1) */
        /* First-order recurrence: sum depends on previous iteration's sum */
        sum = sum * 0.999 + x;  /* Reduction with loop-carried dependency */
        
        /* iii. Conditional store based on previous iteration */
        if (i > 0 && array_b[i-1] > 0.5) {  /* Distance-1 dependency */
            array_c[i] = x * sum;  /* Uses current sum */
        }
        
        /* iv. Access multiple arrays with non-constant offsets */
        /* Complex address calculation prevents optimization */
        int idx = (i + offset) % (SIZE + OFFSET);
        array_b[i] = array_b[i] + array_a[idx] * 0.5;
        
        /* Additional high-latency operation to increase register pressure */
        if (i % 4 == 0) {
            /* Use sin() which has high latency */
            array_a[i] = array_a[i] + sin(array_b[i] * 0.01);
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: sum = %f, array_c[%d] = %f\n", 
           sum, SIZE-1, array_c[SIZE-1]);
    
    /* Additional check to ensure all computations are used */
    double checksum = sum;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
