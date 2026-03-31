#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the target region */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int n, int *sum_ptr) {
    int sum = 0;
    
    /* Target region with SIMD clause - critical for triggering SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) \
                map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        /* Complex conditional with data-dependent branches to create non-trivial CFG */
        volatile int threshold = 50;  /* Prevent constant propagation */
        
        /* First conditional branch - creates basic block structure */
        if (a[i] > threshold) {
            /* Nested conditional to increase CFG complexity */
            if (b[i] < 100) {
                /* Arithmetic operations amenable to vectorization but with masking */
                sum += a[i] * b[i];
            }
        }
        
        /* Additional conditional to ensure multiple basic blocks */
        if (a[i] % 2 == 0) {
            /* More arithmetic to prevent loop simplification */
            sum += a[i] / 2;
        }
    }
    
    *sum_ptr = sum;
}

int main() {
    /* Seed for reproducible results */
    srand(42);
    
    /* Arrays with pseudo-random data */
    int a[SIZE], b[SIZE];
    
    /* Initialize arrays with random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Volatile to prevent compile-time loop bound determination */
    volatile int size = SIZE;
    int sum = 0;
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use result to prevent dead code elimination */
    printf("Result sum = %d\n", sum);
    
    /* Additional volatile store to ensure code generation */
    volatile int dummy = sum;
    
    return 0;
}
