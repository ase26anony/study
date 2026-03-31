#include <stdio.h>
#include <math.h>
#include <stdlib.h>

/* 
 * This program is designed to trigger GCC's modulo scheduler debug output
 * for specific edge moves during backtracking/scheduling.
 * 
 * Key features that force complex modulo scheduling:
 * 1. Loop-carried dependency through 'sum' (distance-1 edge)
 * 2. High-latency operations (sqrt, division)
 * 3. Multiple memory accesses with potential aliasing
 * 4. Conditional store with loop-carried dependency
 * 5. Non-constant array offsets
 */

#define SIZE 1024

/* Use volatile to prevent constant propagation and force memory accesses */
static volatile int offset = 3;

/* Function to prevent loop unrolling and maintain loop structure */
__attribute__((noinline))
double compute_target_loop(double* arr1, double* arr2, double* arr3, int n, int off) {
    double sum = 1.0;
    int i;
    
    /* 
     * Target loop for modulo scheduling
     * This should create complex DDG edges with distance-1 dependencies
     */
    for (i = 0; i < n; i++) {
        /* High-latency operation 1: sqrt with division */
        double x = sqrt(arr1[i] + 1.0) / 2.5;
        
        /* Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.999 + x;
        
        /* Access multiple arrays with non-constant offsets */
        double y = arr2[(i + off) % n] * 1.5;
        
        /* Another high-latency operation */
        double z = y / (arr1[i] + 0.001);
        
        /* Conditional store with loop-carried dependency */
        if (i > 0 && arr3[i-1] > 0.5) {
            arr3[i] = x + z;
        }
        
        /* Additional memory access with potential aliasing */
        arr2[i] = arr2[i] * 0.9 + sum;
        
        /* Mix integer and floating point to create register pressure */
        int temp_int = (int)(sum * 100) % 256;
        arr1[(i + 1) % n] += temp_int * 0.01;
    }
    
    return sum;
}

/* Initialize arrays with pseudo-random but deterministic values */
void init_arrays(double* arr1, double* arr2, double* arr3, int n) {
    int i;
    for (i = 0; i < n; i++) {
        /* Use a simple pseudo-random sequence */
        arr1[i] = (i * 1.2345) / n;
        arr2[i] = sin(i * 0.01) + 1.0;
        arr3[i] = cos(i * 0.005) * 2.0;
    }
}

int main(int argc, char** argv) {
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE];
    
    /* Initialize with deterministic values */
    init_arrays(array_a, array_b, array_c, SIZE);
    
    /* Use command line argument to vary offset slightly */
    int off = offset;
    if (argc > 1) {
        off = atoi(argv[1]) % 10;
    }
    
    /* 
     * Execute the target loop multiple times to ensure
     * the modulo scheduler runs on hot code
     */
    double final_sum = 0.0;
    int iterations = 2;
    
    while (iterations-- > 0) {
        final_sum += compute_target_loop(array_a, array_b, array_c, SIZE, off);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < SIZE; i++) {
            array_a[i] *= 0.99;
            array_b[i] += 0.01;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final checksum: %f\n", final_sum);
    
    /* Also use array_c to prevent elimination */
    double verify = 0.0;
    for (int i = 0; i < SIZE; i++) {
        verify += array_c[i];
    }
    printf("Array C sum: %f\n", verify);
    
    return 0;
}
