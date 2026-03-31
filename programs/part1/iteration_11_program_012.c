#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent interprocedural optimizations */
static void compute_target(int *a, int *b, int size, int *sum) 
    __attribute__((noinline, noipa));

static void compute_target(int *a, int *b, int size, int *sum)
{
    int local_sum = 0;
    volatile int threshold = 50; /* Prevent constant propagation */
    
    /* Complex loop with data-dependent conditionals for SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:local_sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: local_sum)
    for (int i = 0; i < size; i++) {
        /* Multiple data-dependent conditionals to create complex CFG */
        if (a[i] > threshold) {
            local_sum += a[i] * b[i];
        }
        
        /* Additional conditional to increase basic block complexity */
        if (b[i] < threshold && a[i] > threshold / 2) {
            local_sum -= b[i];
        }
        
        /* Another conditional with arithmetic */
        if (a[i] % 5 == 0) {
            local_sum += i;
        }
    }
    
    *sum = local_sum;
}

int main(void)
{
    int a[SIZE], b[SIZE];
    volatile int size = SIZE; /* Prevent compile-time optimization */
    int sum = 0;
    
    /* Initialize with pseudo-random data */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call target function */
    compute_target(a, b, size, &sum);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    /* Additional volatile store to ensure code generation */
    volatile int check = sum;
    
    return 0;
}
