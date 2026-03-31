#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the target region */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int n, int *sum_ptr) {
    int sum = 0;
    volatile int threshold = 50; /* Prevent constant propagation */
    
    /* 
     * Complex OpenMP target region with SIMD clause that should trigger
     * the SIMT transformation in omp-low.cc
     */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        /* 
         * Complex conditional reduction with data-dependent branches
         * Creates non-trivial control flow for SIMT transformation
         */
        if (a[i] > threshold) {
            /* First conditional branch */
            sum += a[i] * b[i];
            
            /* Additional data-dependent operation to create more complex CFG */
            if (b[i] < threshold / 2) {
                /* Nested conditional for more complex masking requirements */
                sum += a[i] - b[i];
            }
        } else if (a[i] < threshold / 3) {
            /* Alternative branch path for SIMT masking */
            sum += b[i] - a[i];
        }
        
        /* Additional arithmetic to ensure loop is non-trivial */
        int temp = a[i] + b[i];
        if (temp % 7 == 0) {
            /* Another conditional to create more basic blocks */
            sum += temp;
        }
    }
    
    *sum_ptr = sum;
}

int main() {
    /* Seed random number generator for reproducible results */
    srand(42);
    
    /* Declare and initialize arrays */
    int a[SIZE], b[SIZE];
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;  /* Values between 0-99 */
        b[i] = rand() % 100;
    }
    
    /* Use volatile to prevent compile-time optimization of loop bounds */
    volatile int size = SIZE;
    int sum = 0;
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result sum = %d\n", sum);
    
    /* Additional volatile store to ensure code isn't optimized away */
    volatile int result_check = sum;
    
    return 0;
}
