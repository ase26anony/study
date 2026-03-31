#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* Volatile variable to prevent constant propagation */
volatile int offset = 1;

/* Function to generate pseudo-random values */
double rand_val(int i) {
    return ((i * 1103515245 + 12345) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    /* Initialize arrays with pseudo-random data */
    double array_a[SIZE + 2];
    double array_b[SIZE + 2];
    double array_c[SIZE + 2];
    
    /* Use argv to create variation and prevent optimization */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    for (int i = 0; i < SIZE + 2; i++) {
        array_a[i] = rand_val(i + seed);
        array_b[i] = rand_val(i + seed * 2);
        array_c[i] = 0.0;
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = array_a[0];
    
    /* Variable to create conditional dependency */
    double prev_condition = array_b[0];
    
    /* The target innermost loop for modulo scheduling */
    for (int i = 1; i < SIZE; i++) {
        /* High latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0);
        
        /* Another high latency operation: division */
        double y = x / (array_b[i] + 0.001);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + y;
        
        /* Access multiple arrays with non-constant offset */
        /* Using volatile 'offset' prevents constant propagation */
        double temp = array_a[i + offset] + array_b[i - offset];
        
        /* Conditional store with loop-carried dependency */
        if (prev_condition > 0.5) {
            array_c[i] = x * temp;
        }
        
        /* Update condition for next iteration (distance-1 dependency) */
        prev_condition = array_b[i] * 0.8 + prev_condition * 0.2;
        
        /* Additional memory access with complex addressing */
        array_a[i + 1] = array_a[i + 1] * 0.9 + sum * 0.1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: sum = %f, array_c[%d] = %f\n", 
           sum, SIZE/2, array_c[SIZE/2]);
    
    return 0;
}
