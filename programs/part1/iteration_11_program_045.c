#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the target region */
static void compute_target(int *a, int *b, int n, int *sum) 
    __attribute__((noinline, noipa));

static void compute_target(int *a, int *b, int n, int *sum)
{
    int local_sum = 0;
    
    /* Use volatile to prevent constant propagation of threshold */
    volatile int threshold = 50;
    
    /* 
     * Complex OpenMP target region with SIMD clause
     * This should trigger the SIMT transformation in omp-low.cc
     */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:local_sum) \
                map(to: a[0:n], b[0:n]) \
                map(tofrom: local_sum)
    for (int i = 0; i < n; i++) {
        /* 
         * Data-dependent conditional with arithmetic operations
         * Creates non-trivial control flow for SIMD/SIMT vectorization
         */
        if (a[i] > threshold) {
            local_sum += a[i] * b[i];
        }
        
        /* Additional conditional to create more complex CFG */
        if (b[i] < threshold / 2) {
            local_sum -= a[i];
        }
        
        /* More arithmetic to prevent loop simplification */
        local_sum += (a[i] % 10) - (b[i] % 10);
    }
    
    *sum = local_sum;
}

int main(void)
{
    int a[SIZE], b[SIZE];
    int sum = 0;
    
    /* Use volatile for size to prevent compile-time loop unrolling */
    volatile int size = SIZE;
    
    /* Seed random number generator for reproducible results */
    srand(42);
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    /* Additional volatile store to ensure code generation */
    volatile int result_check = sum;
    
    return 0;
}
