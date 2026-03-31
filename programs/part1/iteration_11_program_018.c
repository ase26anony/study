#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the OpenMP construct */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int n, int *sum_result)
{
    int sum = 0;
    volatile int threshold = 50; /* Prevent constant propagation */
    
    /* 
     * Combined target construct with SIMD clause.
     * This should trigger the SIMT transformation in omp-low.cc
     * when compiled with OpenMP offloading.
     */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        /* 
         * Data-dependent conditional with arithmetic operations.
         * Creates non-trivial control flow requiring masking for SIMD/SIMT.
         */
        if (a[i] > threshold) {
            sum += a[i] * b[i];
        }
        
        /* Additional conditional to create more complex basic block structure */
        if (b[i] < threshold / 2) {
            sum -= a[i];
        }
        
        /* Another arithmetic operation to increase loop body complexity */
        sum += (a[i] % 10) * (b[i] % 10);
    }
    
    *sum_result = sum;
}

int main(void)
{
    int a[SIZE], b[SIZE];
    volatile int size = SIZE; /* Runtime-determined size to prevent optimization */
    int sum = 0;
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the target function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    return 0;
}
