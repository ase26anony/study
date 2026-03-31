#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define SIZE 1024
#define OFFSET 3

/* Prevent constant propagation and loop unrolling */
static volatile int g_offset = OFFSET;

/* Function to create pseudo-random values */
double rand_val(int i) {
    return (double)((i * 1103515245 + 12345) & 0x7FFFFFFF) / 1073741824.0;
}

int main(int argc, char *argv[]) {
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    double sum = 1.0;
    int i;
    
    /* Initialize arrays with pseudo-random data */
    for (i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = rand_val(i);
        if (i < SIZE) {
            array_b[i] = rand_val(i * 2);
            array_c[i] = 0.0;
        }
    }
    
    /* Use argv to create non-constant loop bound (prevents optimization) */
    int limit = SIZE;
    if (argc > 1) {
        limit = atoi(argv[1]);
        if (limit <= 0 || limit > SIZE) limit = SIZE;
    }
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required patterns:
     */
    for (i = 0; i < limit; i++) {
        /* 1. High-latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0) / (array_b[i] + 0.001);
        
        /* 2. Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + x;  /* True recurrence: sum depends on previous iteration */
        
        /* 3. Conditional store based on previous iteration value */
        if (i > 0 && array_b[i-1] > 0.5) {
            array_c[i] = x * sum;  /* Uses both x and sum from current iteration */
        }
        
        /* 4. Multiple memory accesses with non-constant offsets */
        /* Access array_a with offset to create complex addressing */
        double temp = array_a[i + g_offset] * 0.5;
        
        /* 5. Additional high-latency operation: sine function */
        array_b[i] = sin(temp) + (i > 0 ? array_c[i-1] * 0.1 : 0.0);
        
        /* 6. Another recurrence with different distance */
        if (i >= 2) {
            array_a[i] = array_a[i] + array_a[i-2] * 0.3;  /* Distance-2 dependency */
        }
    }
    
    /* Prevent dead code elimination */
    printf("Result: sum = %f, array_c[%d] = %f\n", 
           sum, limit-1, array_c[limit-1]);
    
    /* Use the results to prevent optimization */
    double checksum = sum;
    for (i = 0; i < limit; i++) {
        checksum += array_c[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return (int)(checksum * 1000) % 256;
}
