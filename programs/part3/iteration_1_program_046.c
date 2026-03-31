#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Prevent compiler from optimizing away the loop */
volatile int offset = 1;

/* Function to generate pseudo-random values */
double rand_val(int i) {
    return ((i * 1103515245 + 12345) & 0x7fffffff) / 2147483647.0;
}

int main(int argc, char *argv[]) {
    const int N = 1024;
    double array_a[N], array_b[N], array_c[N];
    double sum = 0.0;
    int i;
    
    /* Initialize arrays with pseudo-random values */
    for (i = 0; i < N; i++) {
        array_a[i] = rand_val(i) * 10.0;
        array_b[i] = rand_val(i * 3) * 5.0 - 2.5;
        array_c[i] = 0.0;
    }
    
    /* Use command line argument to create variability */
    int base_offset = (argc > 1) ? atoi(argv[1]) % 4 : 1;
    
    /* 
     * TARGET LOOP for modulo scheduling
     * This loop contains all the required patterns:
     */
    for (i = 0; i < N - 2; i++) {
        /* 1. High latency operation: sqrt with division */
        double x = sqrt(array_a[i] + 1.0) / 1.5;
        
        /* 2. Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.99 + x;  /* True recurrence: sum depends on previous iteration's sum */
        
        /* 3. Multiple memory accesses with non-constant offsets */
        /* Create complex addressing to prevent optimization */
        double temp1 = array_a[i + base_offset];
        double temp2 = array_b[i + offset];  /* volatile offset creates uncertainty */
        
        /* 4. Conditional store with loop-carried dependency */
        if (i > 0) {
            /* Condition depends on value from previous iteration */
            if (array_b[i-1] > 0.0) {  /* Distance-1 memory dependency */
                array_c[i] = x * temp1 + temp2;
            }
        }
        
        /* Additional high-latency operation to increase pressure */
        array_b[i] = array_a[i] / (temp1 + 0.001);  /* Division has variable latency */
    }
    
    /* Prevent dead code elimination */
    printf("Result: sum = %f, array_c[100] = %f\n", sum, array_c[100]);
    
    return (int)(sum * 1000) % 256;
}
