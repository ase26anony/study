#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* Helper function to generate pseudo-random values */
static double simple_rand(int seed) {
    return ((seed * 1103515245 + 12345) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    double sum = 0.0;
    int i;
    
    /* Use argv to create non-constant values */
    int offset = (argc > 1) ? (argv[1][0] % 4) : 1;
    volatile int k = offset; /* Prevent constant propagation */
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < SIZE; i++) {
        array_a[i] = simple_rand(i);
        array_b[i] = simple_rand(i + 1000);
    }
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required patterns:
     */
    for (i = 0; i < SIZE - 1; i++) {
        /* 1. High latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0);
        double y = x / (array_b[i] + 0.5); /* Non-constant divisor */
        
        /* 2. Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.99 + y; /* True recurrence: sum depends on previous iteration's sum */
        
        /* 3. Multiple memory accesses with potential aliasing */
        /* Access with non-constant offset (k is volatile) */
        double temp = array_a[(i + k) % SIZE] + array_b[(i + 2) % SIZE];
        
        /* 4. Conditional store with loop-carried dependency */
        if (i > 0 && array_b[i-1] > 0.5) { /* Depends on previous iteration's array_b */
            array_c[i] = x * temp; /* Complex store operation */
        }
        
        /* Additional memory access to create more pressure */
        array_b[i] = array_b[i] * 0.9 + sum * 0.1;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %f\n", sum + array_c[SIZE/2]);
    
    return 0;
}
