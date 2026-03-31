#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024
#define OFFSET 3

/* Prevent compiler from optimizing away the loop */
volatile int g_offset = OFFSET;

/* Function to create pseudo-random values */
double pseudo_rand(int i) {
    return ((i * 1103515245 + 12345) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    /* Use argc to create some variability in initial values */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Declare arrays with different types to avoid type-based aliasing assumptions */
    double array_a[SIZE + OFFSET];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < SIZE + OFFSET; i++) {
        array_a[i] = pseudo_rand(i + seed) * 100.0 + 1.0;
        if (i < SIZE) {
            array_b[i] = pseudo_rand(i + seed + 1000) * 50.0 - 25.0;
            array_c[i] = 0.0;
        }
    }
    
    /* Reduction variable with loop-carried dependency */
    double sum = array_a[0] * 0.5;
    
    /* Variable to create conditional dependency across iterations */
    double prev_condition = array_b[0];
    
    /* The target innermost loop for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* High-latency operation: sqrt with division */
        double x = sqrt(array_a[i + g_offset] + 1.0);
        x = x / (array_a[i] + 0.001);  /* Non-constant divisor */
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.99 + x;
        
        /* Access multiple arrays with non-constant offsets */
        double temp = array_b[i] + array_a[(i + 1) % (SIZE + OFFSET)];
        
        /* Conditional store with loop-carried dependency */
        if (i > 0 && prev_condition > 0.0) {
            array_c[i] = x * temp;
        }
        
        /* Update condition for next iteration (distance-1 dependency) */
        prev_condition = array_b[i] * 0.8 + prev_condition * 0.2;
        
        /* Another high-latency operation to increase register pressure */
        if (i % 4 == 0) {
            double y = sin(array_a[i + g_offset] * 0.01);
            sum += y * 0.1;
        }
        
        /* Additional memory access with complex addressing */
        int idx = (i * 7) % (SIZE + OFFSET);
        array_a[idx] = array_a[idx] * 0.999 + x * 0.001;
    }
    
    /* Use the results to prevent dead code elimination */
    double checksum = sum;
    for (int i = 0; i < SIZE; i++) {
        checksum += array_c[i] * 0.01;
    }
    
    printf("Checksum: %f\n", checksum);
    return (int)(checksum * 1000) % 256;
}
