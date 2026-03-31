#include <stdio.h>
#include <stdlib.h>

/* Prevent interprocedural optimizations from removing the target region */
static void compute_target(const int *a, const int *b, int size, int *sum) 
    __attribute__((noinline, noipa));

static void compute_target(const int *a, const int *b, int size, int *sum) {
    /* Volatile threshold to prevent constant propagation */
    volatile int threshold = 50;
    int thresh = threshold;
    
    /* 
     * Combined target teams distribute parallel for simd construct.
     * The 'simd' clause is crucial for triggering SIMT transformations.
     * Reduction with conditional inside loop creates control flow for masking.
     */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:*sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: *sum)
    for (int i = 0; i < size; ++i) {
        /* 
         * Data-dependent conditional with arithmetic operations.
         * Multiple conditionals create the basic block structure needed
         * for SIMT transformation in omp-low.cc
         */
        if (a[i] > thresh) {
            /* Complex enough operation to prevent scalar replacement */
            int temp = a[i] * b[i];
            
            /* Additional conditional to create more control flow */
            if (temp > 0) {
                *sum += temp;
            } else {
                /* Alternative path to ensure both branches exist */
                *sum += a[i] + b[i];
            }
        } else if (a[i] < -thresh) {
            /* Another conditional path for richer control flow */
            *sum -= b[i];
        }
    }
}

int main() {
    const int N = 1024;
    int a[N], b[N];
    
    /* Seed for reproducible pseudo-random data */
    srand(42);
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < N; ++i) {
        a[i] = rand() % 100 - 25;  /* Range: -25 to 74 */
        b[i] = rand() % 100 - 25;
    }
    
    /* Volatile size to prevent compile-time loop unrolling/optimization */
    volatile int vsize = N;
    int size = vsize;
    
    int sum = 0;
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use result to prevent dead code elimination */
    printf("Result sum = %d\n", sum);
    
    return 0;
}
