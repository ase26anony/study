#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* Prevent compiler from optimizing away the loop */
volatile int offset = 1;

/* Function to generate pseudo-random values */
double rand_val(int i) {
    return (double)((i * 1103515245 + 12345) & 0x7fffffff) / 0x7fffffff;
}

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = rand_val(i);
        array_b[i] = rand_val(i + SIZE);
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Use argv to create non-constant offset */
    int k = (argc > 1) ? atoi(argv[1]) % 4 : 1;
    if (k == 0) k = 1;
    
    /* The target innermost loop for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* High latency operation: sqrt with division */
        double x;
        if (i + k < SIZE) {
            /* Complex address calculation with non-constant offset */
            x = sqrt(array_a[i + k] + 1.0) / (array_b[i] + 0.001);
        } else {
            x = sqrt(array_a[i] + 1.0) / (array_b[i] + 0.001);
        }
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + x;
        
        /* Conditional store with loop-carried dependency */
        if (i > 0) {
            /* Dependency on previous iteration's value */
            if (array_b[i-1] > 0.5) {
                /* High latency operation in conditional path */
                array_c[i] = sin(x) * array_c[i-1];
            }
        }
        
        /* Additional memory access with potential aliasing */
        array_b[i] = array_b[i] * 0.9 + sum * 0.1;
        
        /* Another high latency operation */
        if (i % 3 == 0) {
            array_a[i] = array_a[i] / (fabs(sum) + 1.0);
        }
    }
    
    /* Prevent dead code elimination */
    printf("Result: %f\n", sum);
    
    /* Use result to affect return value */
    return (sum > 0) ? 0 : 1;
}
