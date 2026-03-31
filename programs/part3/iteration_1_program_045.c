#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* 
 * This function creates a loop that should trigger GCC's modulo scheduler
 * to generate the specific debug output in modulo-sched.cc lines 596-606
 */
double target_loop(double* arr1, double* arr2, double* arr3, int n, int offset) {
    double sum = 1.0;
    double prev_condition = 0.0;
    
    /* 
     * Innermost loop with carried dependencies - this is what the modulo
     * scheduler will attempt to pipeline
     */
    for (int i = 0; i < n; i++) {
        /* High latency operation: sqrt with floating point division */
        double x = sqrt(arr1[i] + 1.0) / (arr2[i] + 0.001);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.99 + x;
        
        /* Conditional store with carried dependency */
        if (i > 0 && prev_condition > 0.5) {
            arr3[i] = x * sum;
        }
        
        /* Update condition for next iteration (distance-1 dependency) */
        prev_condition = arr2[i];
        
        /* Multiple memory accesses with potential aliasing */
        /* Using non-constant offset to prevent optimization */
        double temp = arr1[(i + offset) % n] * arr2[(i + 1) % n];
        
        /* Another high latency operation */
        arr1[i] = temp / (sum + 1.0);
    }
    
    return sum;
}

int main(int argc, char** argv) {
    /* Initialize arrays with pseudo-random values */
    double* array_a = (double*)malloc(SIZE * sizeof(double));
    double* array_b = (double*)malloc(SIZE * sizeof(double));
    double* array_c = (double*)malloc(SIZE * sizeof(double));
    
    if (!array_a || !array_b || !array_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Use argv to create non-constant offset for memory accesses */
    int offset = (argc > 1) ? atoi(argv[1]) % 10 : 3;
    
    /* Initialize with values that create varied execution paths */
    for (int i = 0; i < SIZE; i++) {
        /* Create some pattern to ensure conditional executes */
        array_a[i] = (i % 7) * 0.3;
        array_b[i] = (i % 5) * 0.4;
        array_c[i] = 0.0;
    }
    
    /* Execute the target loop multiple times to ensure it's hot */
    double final_sum = 0.0;
    for (int iter = 0; iter < 10; iter++) {
        final_sum += target_loop(array_a, array_b, array_c, SIZE, offset);
        
        /* Modify arrays slightly between iterations to prevent 
           complete optimization */
        for (int i = 0; i < SIZE; i++) {
            array_a[i] += 0.01;
            array_b[i] -= 0.005;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %f\n", final_sum);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    
    return 0;
}
