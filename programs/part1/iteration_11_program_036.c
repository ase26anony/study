/* This program is designed to trigger the SIMT transformation in GCC's omp-low.cc
   Specifically targeting lines 2941-2975 which handle IFN_GOMP_USE_SIMT generation */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent constant propagation and loop unrolling */
static volatile int threshold = 50;

/* Function containing the target region - marked to prevent optimization */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int size, int *sum_result) {
    int sum = 0;
    
    /* Complex OpenMP target construct with SIMD clause */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: sum) \
                num_teams(8) thread_limit(256)
    for (int i = 0; i < size; i++) {
        /* Data-dependent conditional branch - creates control flow for SIMT */
        if (a[i] > threshold) {
            /* Nested condition to create more complex CFG */
            if (b[i] != 0) {
                /* Arithmetic operations suitable for vectorization */
                int temp = a[i] * b[i];
                /* Another conditional to create basic block structure */
                if (temp > 1000) {
                    sum += temp / 2;
                } else {
                    sum += temp;
                }
            }
        } else if (a[i] < threshold / 2) {
            /* Alternative path with different computation */
            sum -= b[i];
        }
        
        /* Additional operation to prevent loop simplification */
        volatile int dummy = a[i] + b[i];
        (void)dummy;  /* Suppress unused warning */
    }
    
    *sum_result = sum;
}

int main() {
    /* Seed random number generator for reproducible results */
    srand(42);
    
    /* Use volatile size to prevent compile-time loop bound determination */
    volatile int size = 1024;
    int actual_size = size;
    
    /* Allocate and initialize arrays with pseudo-random data */
    int *a = (int*)malloc(actual_size * sizeof(int));
    int *b = (int*)malloc(actual_size * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    for (int i = 0; i < actual_size; i++) {
        a[i] = rand() % 100;      /* Values 0-99 */
        b[i] = rand() % 100 + 1;  /* Values 1-100 (avoid zero for division) */
    }
    
    int sum = 0;
    
    /* Call the target computation function */
    compute_target(a, b, actual_size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result sum: %d\n", sum);
    
    /* Additional volatile store to ensure code generation */
    volatile int result_check = sum;
    (void)result_check;
    
    free(a);
    free(b);
    
    return 0;
}
