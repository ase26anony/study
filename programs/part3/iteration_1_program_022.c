#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent compiler from optimizing away the loop */
volatile int g_offset = 1;

/* Function to generate pseudo-random values */
double rand_val(int i, int seed) {
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
        array_a[i] = rand_val(i, seed);
        array_b[i] = rand_val(i, seed + 1);
        array_c[i] = 0.0;
    }
    
    double sum = 0.0;
    int offset = g_offset;  /* Volatile prevents constant propagation */
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required patterns:
     */
    for (int i = 0; i < N; i++) {
        /* 1. High-latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0);
        
        /* 2. Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.99 + x;  /* True recurrence: sum depends on previous iteration's sum */
        
        /* 3. Conditional store with carried dependency */
        if (i > 0 && array_b[i-1] > 0.5) {  /* Distance-1 memory dependency */
            array_c[i] = x * array_b[i-1];  /* Uses value from previous iteration */
        }
        
        /* 4. Multiple memory accesses with non-constant offsets */
        /* Access with offset creates complex address calculation */
        int idx = (i + offset) % N;
        double temp = array_a[idx] / (array_b[i] + 0.001);  /* High-latency division */
        
        /* Additional carried dependency through array_b */
        array_b[i] = array_b[i] * 0.8 + temp * 0.2;
        
        /* Another high-latency operation to increase pressure */
        if (i % 3 == 0) {
            array_a[i] = sin(array_a[i]);  /* High-latency function call */
        }
    }
    
    /* Use the results to prevent dead code elimination */
    printf("Result: sum = %f, array_c[%d] = %f\n", 
           sum, N-1, array_c[N-1]);
    
    /* Additional computation to ensure all values are used */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        checksum += array_a[i] + array_b[i] + array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return (int)(sum + checksum) % 100;
}
