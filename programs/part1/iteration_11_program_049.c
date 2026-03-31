#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the target region */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int n, int *sum_ptr)
{
    int sum = 0;
    volatile int threshold = 50; /* Prevent constant propagation */
    
    /* Complex OpenMP target region with SIMD clause */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        /* Data-dependent conditional branch - creates non-trivial CFG */
        if (a[i] > threshold) {
            /* Nested condition to create more complex control flow */
            if (b[i] != 0) {
                sum += a[i] * b[i];
            } else {
                /* Alternative path to ensure both branches exist */
                sum += a[i];
            }
        } else {
            /* Another conditional branch in the else path */
            if (a[i] < threshold / 2) {
                sum -= b[i];
            }
        }
        
        /* Additional arithmetic to create more IL for vectorization */
        int temp = a[i] + b[i];
        if (temp % 3 == 0) {
            /* Another conditional inside the loop */
            sum += temp;
        }
    }
    
    *sum_ptr = sum;
}

int main(void)
{
    int a[SIZE], b[SIZE];
    volatile int size = SIZE; /* Prevent compile-time loop unrolling */
    int sum = 0;
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    /* Additional volatile store to ensure code isn't optimized away */
    volatile int check = sum;
    (void)check;
    
    return 0;
}
