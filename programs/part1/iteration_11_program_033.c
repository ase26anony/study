#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the target region */
static void compute_target(int *a, int *b, int size, int *sum) 
    __attribute__((noinline, noipa));

static void compute_target(int *a, int *b, int size, int *sum)
{
    int local_sum = 0;
    
    /* 
     * Complex OpenMP target region with SIMD clause.
     * The volatile threshold prevents constant propagation.
     */
    volatile int threshold = 50;
    
    #pragma omp target teams distribute parallel for simd \
                reduction(+:local_sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: local_sum)
    for (int i = 0; i < size; i++) {
        /* 
         * Data-dependent conditional with arithmetic operations.
         * Creates non-trivial control flow requiring masking for SIMD/SIMT.
         */
        if (a[i] > threshold) {
            local_sum += a[i] * b[i];
            
            /* Additional conditional to create more complex CFG */
            if (b[i] % 2 == 0) {
                local_sum += a[i];
            } else {
                local_sum -= b[i];
            }
        } else if (a[i] < threshold / 2) {
            /* Another conditional path for more complexity */
            local_sum += b[i] - a[i];
        }
        
        /* Additional arithmetic to prevent loop simplification */
        local_sum += (i % 8) * 2;
    }
    
    *sum = local_sum;
}

int main(void)
{
    int a[SIZE], b[SIZE];
    int sum = 0;
    
    /* Use volatile for size to prevent compile-time optimization */
    volatile int size = SIZE;
    
    /* Seed random number generator for reproducible results */
    srand(42);
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;  /* Values between 0-99 */
        b[i] = rand() % 100;
    }
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result sum = %d\n", sum);
    
    /* Additional volatile store to ensure code isn't optimized away */
    volatile int check = sum;
    (void)check;
    
    return 0;
}
