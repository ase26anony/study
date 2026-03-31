#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent compiler from optimizing away the loop */
volatile int offset = 1;

/* Function to generate pseudo-random values */
static inline double pseudo_rand(int i, int seed) {
    return ((i * 1103515245 + 12345 + seed) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    const int N = 1024;
    double array_a[N];
    double array_b[N];
    double array_c[N];
    
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array_a[i] = pseudo_rand(i, seed);
        array_b[i] = pseudo_rand(i * 2, seed + 1);
        array_c[i] = 0.0;
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Variable to create conditional dependency across iterations */
    double prev_condition = 0.0;
    
    /* The target innermost loop for modulo scheduling */
    for (int i = 0; i < N - offset; i++) {
        /* High-latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0);
        
        /* Another high-latency operation: division with non-constant divisor */
        double y = x / (array_b[i] + 0.001);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + y;
        
        /* Access multiple arrays with non-constant offsets */
        double temp_a = array_a[i + offset];
        double temp_b = array_b[(i * 3) % N];
        
        /* Conditional store with loop-carried dependency */
        if (i > 0 && prev_condition > 0.5) {
            array_c[i] = x * temp_a;
        }
        
        /* Update condition for next iteration (distance-1 dependency) */
        prev_condition = temp_b * sum;
        
        /* Additional memory access with potential aliasing */
        array_b[i] = array_b[i] * 0.9 + sum * 0.1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: sum = %.10f, array_c[%d] = %.10f\n", 
           sum, N/2, array_c[N/2]);
    
    return 0;
}
