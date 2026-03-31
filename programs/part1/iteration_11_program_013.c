#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations */
static void compute_target(int *a, int *b, int size, int *sum) 
    __attribute__((noinline, noipa));

static void compute_target(int *a, int *b, int size, int *sum)
{
    int local_sum = 0;
    volatile int threshold = 50; /* Prevent constant propagation */
    
    /* Target region with SIMD clause - should trigger SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
        reduction(+:local_sum) \
        map(to: a[0:size], b[0:size]) \
        map(tofrom: local_sum)
    for (int i = 0; i < size; i++) {
        /* Complex conditional with data-dependent branches */
        if (a[i] > threshold) {
            local_sum += a[i] * b[i];
        }
        
        /* Additional conditional to create more complex CFG */
        if (b[i] < threshold / 2) {
            local_sum -= a[i];
        }
        
        /* Nested conditional for more complex control flow */
        if (a[i] % 2 == 0) {
            if (b[i] % 3 == 0) {
                local_sum += 1;
            } else {
                local_sum -= 1;
            }
        }
    }
    
    *sum = local_sum;
}

int main(void)
{
    int a[SIZE], b[SIZE];
    int sum = 0;
    volatile int size = SIZE; /* Prevent compile-time optimization */
    
    /* Seed random number generator */
    srand(42);
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call target function */
    compute_target(a, b, size, &sum);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    /* Additional volatile store to ensure code isn't optimized away */
    volatile int dummy = sum;
    
    return 0;
}
