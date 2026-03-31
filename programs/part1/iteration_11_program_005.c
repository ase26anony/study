#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the OpenMP construct */
static void compute_target(int *a, int *b, int n, int *sum) 
    __attribute__((noinline, noipa));

static void compute_target(int *a, int *b, int n, int *sum)
{
    int local_sum = 0;
    volatile int threshold = 50; /* Prevent constant propagation */
    
    /* 
     * Combined target teams distribute parallel for simd construct
     * This should trigger the SIMT transformation in omp-low.cc
     * when compiled with OpenMP offloading
     */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:local_sum) \
                map(to: a[0:n], b[0:n]) \
                map(tofrom: local_sum)
    for (int i = 0; i < n; i++) {
        /* 
         * Complex loop body with data-dependent conditional branches
         * Creates non-trivial control flow graph requiring masking
         * for SIMD/SIMT vectorization
         */
        if (a[i] > threshold) {
            /* First conditional branch */
            local_sum += a[i] * b[i];
            
            /* Additional data-dependent operation to increase complexity */
            if (b[i] < threshold / 2) {
                /* Nested conditional for more complex CFG */
                local_sum -= a[i];
            }
        } else if (a[i] < threshold / 3) {
            /* Alternative conditional path */
            local_sum += b[i];
        }
        
        /* Another independent conditional */
        if ((a[i] + b[i]) % 2 == 0) {
            local_sum += 1;
        }
    }
    
    *sum = local_sum;
}

int main(void)
{
    int a[SIZE], b[SIZE];
    int sum = 0;
    volatile int size = SIZE; /* Prevent compile-time loop unrolling */
    
    /* Seed random number generator for reproducible results */
    srand(42);
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;  /* Values 0-99 */
        b[i] = rand() % 100;
    }
    
    /* Call the target offloading function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    volatile int result = sum; /* Force memory store */
    printf("Result: %d\n", result);
    
    return 0;
}
