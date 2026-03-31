#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent constant propagation and loop unrolling */
volatile int offset = 1;

/* Function to prevent inlining and create register pressure */
__attribute__((noinline))
double compute_value(double a, double b, int iter) {
    /* High latency operation with data-dependent branching */
    double result;
    if (iter % 3 == 0) {
        result = sqrt(a + 1.0);  /* High latency sqrt */
    } else if (iter % 3 == 1) {
        result = a / (b + 0.5);  /* High latency division */
    } else {
        result = sin(a) * cos(b); /* Function calls with potential high latency */
    }
    return result;
}

int main(int argc, char *argv[]) {
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    double sum = 0.0;
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (i * 3.14159) / (i + 1.0);
        array_b[i] = (i % 7) * 0.5 + 0.1;
    }
    
    /* Get offset from command line to prevent constant propagation */
    if (argc > 1) {
        offset = atoi(argv[1]) % 5;
    }
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop has all the characteristics needed:
     * 1. Loop-carried dependency through 'sum'
     * 2. High latency operations
     * 3. Multiple memory accesses with potential aliasing
     * 4. Conditional store with cross-iteration dependency
     */
    for (int i = 0; i < SIZE; i++) {
        /* High latency operation with data-dependent computation */
        double x = compute_value(array_a[i], array_b[i], i);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + x;  /* This creates a true distance-1 edge */
        
        /* Multiple array accesses with non-constant offset */
        double temp1 = array_a[(i + offset) % SIZE];
        double temp2 = array_b[(i + offset * 2) % SIZE];
        
        /* Additional high-latency operation */
        double y = temp1 / (temp2 + 0.001);  /* Another division */
        
        /* Conditional store with cross-iteration dependency */
        if (i > 0) {
            /* Dependency on previous iteration's array_c value */
            if (array_c[i-1] > 0.5) {
                array_c[i] = x * y + sum;
            } else {
                array_c[i] = x - y;
            }
        } else {
            array_c[i] = x;
        }
        
        /* Additional computation to create more scheduling constraints */
        array_a[i] = array_a[i] * 0.9 + sum * 0.1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: sum = %f, array_c[%d] = %f\n", 
           sum, SIZE-1, array_c[SIZE-1]);
    
    return (int)(sum * 1000) % 100;
}
