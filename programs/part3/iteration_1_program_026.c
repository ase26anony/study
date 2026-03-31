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
    /* Initialize arrays with pseudo-random values */
    double array_a[SIZE + 2];
    double array_b[SIZE + 2];
    double array_c[SIZE + 2];
    
    for (int i = 0; i < SIZE + 2; i++) {
        array_a[i] = rand_val(i);
        array_b[i] = rand_val(i + 1000);
        array_c[i] = 0.0;
    }
    
    /* Use argv to create non-constant offset */
    int k = (argc > 1) ? atoi(argv[1]) % 3 : 1;
    if (k == 0) k = 1;
    
    /* Reduction variable with loop-carried dependency */
    double sum = 1.0;
    
    /* Main loop - target for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation: sqrt with division */
        double x = sqrt(array_a[i + k] + 1.0);
        double y = x / (array_b[i] + 0.001);  /* Division prevents optimization */
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + y;  /* True recurrence: sum depends on previous iteration */
        
        /* Conditional store with loop-carried dependency */
        if (i > 0 && array_b[i - 1] > 0.5) {
            array_c[i] = x * sum;  /* Depends on sum from current iteration */
        }
        
        /* Additional memory access with non-constant offset */
        double temp = array_a[i + offset] * array_b[i];
        
        /* Another high-latency operation */
        array_b[i + 1] = sin(temp) * 0.5;  /* Creates store with potential aliasing */
        
        /* Complex addressing to prevent optimization */
        int idx = (i + k) % (SIZE - 1);
        array_a[idx] = array_a[idx] * 0.99 + array_c[i];
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: sum = %.6f, array_c[%d] = %.6f\n", 
           sum, SIZE/2, array_c[SIZE/2]);
    
    /* Additional computation using results */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i] * (i % 7);
    }
    printf("Checksum: %.6f\n", checksum);
    
    return 0;
}
