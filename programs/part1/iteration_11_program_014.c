#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the target region */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int n, int *sum_ptr) {
    int sum = 0;
    
    /* OpenMP target region with SIMD clause - designed to trigger SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        /* Use volatile threshold to prevent constant propagation */
        volatile int threshold = 50;
        int local_threshold = threshold;
        
        /* Complex conditional with data-dependent branches - creates control flow for SIMT */
        if (a[i] > local_threshold) {
            /* Nested conditional to create more basic blocks */
            if (b[i] < 100) {
                sum += a[i] * b[i];
            } else {
                sum += a[i] + b[i];
            }
        } else if (a[i] < 20) {
            /* Alternative path with different computation */
            sum += b[i] - a[i];
        }
        
        /* Additional arithmetic to make loop body non-trivial */
        int temp = a[i] ^ b[i];
        if (temp & 1) {
            /* Another conditional branch */
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
        a[i] = rand() % 100;  /* Values 0-99 */
        b[i] = rand() % 100;
    }
    
    /* Use volatile for loop bound to prevent compile-time optimization */
    volatile int size = SIZE;
    int sum = 0;
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    /* Additional volatile store to ensure code isn't optimized away */
    volatile int result_check = sum;
    
    return 0;
}
