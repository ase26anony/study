#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* 
 * This function creates a loop that should trigger modulo scheduling
 * with distance-1 dependencies, high-latency operations, and complex
 * memory access patterns.
 */
double target_loop(double* arr1, double* arr2, double* arr3, int n, int offset) {
    double sum = 1.0;
    double prev_cond = 0.0;
    
    /* 
     * This is the target innermost loop for modulo scheduling.
     * It contains all the required patterns:
     * 1. Loop-carried dependency (reduction on 'sum')
     * 2. High-latency operations (sqrt, division)
     * 3. Multiple memory accesses with potential aliasing
     * 4. Conditional store based on previous iteration
     */
    for (int i = 0; i < n; i++) {
        /* High-latency operation 1: square root */
        double x = sqrt(arr1[i] + 1.0);
        
        /* High-latency operation 2: floating-point division */
        double y = x / (arr2[i] + 0.001);
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + y;
        
        /* Access with non-constant offset (creates complex addressing) */
        double offset_val = arr1[(i + offset) % n];
        
        /* Conditional store based on previous iteration's value */
        if (i > 0 && prev_cond > 0.5) {
            arr3[i] = y * offset_val;
        }
        
        /* Update condition for next iteration (distance-1 dependency) */
        prev_cond = arr2[i] * 0.5;
        
        /* Another high-latency operation mixed with memory access */
        arr1[i] = arr1[i] / (sum + 1.0);
    }
    
    return sum;
}

/* 
 * Helper function to initialize arrays with pseudo-random values
 * to avoid constant propagation optimizations
 */
void init_arrays(double* arr1, double* arr2, double* arr3, int n, int seed) {
    for (int i = 0; i < n; i++) {
        /* Use a simple pseudo-random pattern based on seed and index */
        double val = (double)((i * 1103515245 + seed) % 1000) / 100.0;
        arr1[i] = val + 1.0;
        arr2[i] = val * 0.5 + 0.1;
        arr3[i] = 0.0;
    }
}

int main(int argc, char** argv) {
    const int n = SIZE;
    double arr1[SIZE];
    double arr2[SIZE];
    double arr3[SIZE];
    
    /* Use command-line argument to vary initialization (prevents constant folding) */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize arrays with pseudo-random values */
    init_arrays(arr1, arr2, arr3, n, seed);
    
    /* 
     * Use a volatile variable for offset to prevent compiler from 
     * optimizing away the complex addressing pattern
     */
    volatile int offset = 3;
    
    /* Execute the target loop */
    double result = target_loop(arr1, arr2, arr3, n, offset);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", result);
    
    /* Also print a checksum of arr3 to ensure conditional stores executed */
    double checksum = 0.0;
    for (int i = 0; i < n; i++) {
        checksum += arr3[i];
    }
    printf("Checksum: %f\n", checksum);
    
    return 0;
}
