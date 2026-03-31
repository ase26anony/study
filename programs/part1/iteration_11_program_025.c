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
    
    /* 
     * Combined target teams distribute parallel for simd construct
     * This should trigger SIMT transformation for GPU offloading
     */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        /* Complex conditional with data-dependent branches */
        if (a[i] > threshold) {
            /* Nested condition to create more complex CFG */
            if (b[i] > 0) {
                sum += a[i] * b[i];
            } else {
                sum += a[i] - b[i];
            }
        } else if (a[i] < -threshold) {
            /* Another branch to ensure non-trivial control flow */
            sum += b[i] - a[i];
        } else {
            /* Default path with arithmetic operations */
            sum += a[i] + b[i];
        }
        
        /* Additional arithmetic to prevent loop simplification */
        int temp = a[i] * 2;
        if (temp % 3 == 0) {
            sum += 1;
        }
    }
    
    *sum_ptr = sum;
}

int main(void)
{
    /* Seed random number generator for reproducible results */
    srand(42);
    
    /* Declare and initialize arrays */
    int a[SIZE], b[SIZE];
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 200 - 100;  /* Values between -100 and 99 */
        b[i] = rand() % 200 - 100;
    }
    
    /* Use volatile to prevent compile-time optimization of loop bound */
    volatile int size = SIZE;
    int sum = 0;
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result sum = %d\n", sum);
    
    /* Additional check to ensure code isn't optimized away */
    volatile int dummy = sum;
    
    return 0;
}
