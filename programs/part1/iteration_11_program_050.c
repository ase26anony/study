#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the OpenMP construct */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int size, int *sum_ptr) {
    int sum = 0;
    
    /* 
     * Complex OpenMP target region with SIMD clause.
     * The conditional reduction inside creates data-dependent control flow
     * that should trigger SIMT transformation in omp-low.cc
     */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: sum)
    for (int i = 0; i < size; i++) {
        /* 
         * Use volatile threshold to prevent constant propagation.
         * Multiple conditionals create complex control flow for SIMT.
         */
        volatile int threshold = 50;
        int val_a = a[i];
        int val_b = b[i];
        
        /* First conditional branch - creates basic block structure */
        if (val_a > threshold) {
            /* Nested conditional for more complex CFG */
            if (val_b > 0) {
                sum += val_a * val_b;
            } else {
                sum += val_a;
            }
        } else if (val_a > threshold / 2) {
            /* Alternative path with arithmetic */
            sum += val_b - val_a;
        }
        
        /* Additional operation to prevent loop simplification */
        sum += (val_a & 0x1);  /* Parity check */
    }
    
    *sum_ptr = sum;
}

int main() {
    /* Seed for reproducible results */
    srand(42);
    
    /* Arrays with pseudo-random data */
    int a[SIZE], b[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Use volatile for size to prevent compile-time optimization */
    volatile int size = SIZE;
    int sum = 0;
    
    /* Call the target function */
    compute_target(a, b, size, &sum);
    
    /* Use result to prevent dead code elimination */
    printf("Result sum = %d\n", sum);
    
    /* Additional volatile store to ensure code generation */
    volatile int dummy = sum;
    
    return 0;
}
