#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the target region */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int n, int *sum_ptr)
{
    int sum = 0;
    volatile int threshold = 50; /* Prevent constant propagation */
    
    /* Target region with SIMD clause - should trigger SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        /* Complex conditional with data-dependent branches */
        if (a[i] > threshold) {
            /* Nested condition to create more complex CFG */
            if (b[i] < 100) {
                sum += a[i] * b[i];
            } else {
                sum += a[i] + b[i];
            }
        } else if (a[i] > (threshold / 2)) {
            /* Alternative path with arithmetic */
            sum += a[i] - b[i];
        }
        
        /* Additional operation to prevent loop simplification */
        volatile int temp = a[i] ^ b[i];
        (void)temp; /* Suppress unused warning */
    }
    
    *sum_ptr = sum;
}

int main(void)
{
    int a[SIZE], b[SIZE];
    int sum = 0;
    volatile int size = SIZE; /* Runtime-like bound */
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the target function */
    compute_target(a, b, size, &sum);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    /* Additional volatile store as backup */
    volatile int result_check = sum;
    (void)result_check;
    
    return 0;
}
