/* This program is designed to trigger the SIMT transformation in GCC's omp-low.cc
   Specifically targeting lines 2941-2975 which handle IFN_GOMP_USE_SIMT */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int g_size = SIZE;

/* Function containing the target region - marked to prevent optimization */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int *sum, int size) {
    int local_sum = 0;
    
    /* Complex OpenMP target construct with SIMD clause */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:local_sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: local_sum) \
                num_teams(4) thread_limit(128)
    for (int i = 0; i < size; i++) {
        /* Data-dependent conditional branch - creates non-trivial CFG */
        volatile int threshold = 50; /* volatile prevents constant propagation */
        
        /* Multiple conditionals to create complex control flow */
        if (a[i] > threshold) {
            /* Nested conditional for additional complexity */
            if (b[i] < 100) {
                local_sum += a[i] * b[i];
            } else {
                local_sum += a[i] + b[i];
            }
        } else if (a[i] > 25) {
            /* Alternative path with different computation */
            local_sum += b[i] - a[i];
        } else {
            /* Third path with bitwise operations */
            local_sum += a[i] & b[i];
        }
        
        /* Additional arithmetic to create more ILP opportunities */
        local_sum += (i % 16) * 2;
    }
    
    *sum = local_sum;
}

int main() {
    /* Initialize with deterministic random values */
    srand(42);
    
    int a[SIZE], b[SIZE];
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    int sum = 0;
    
    /* Call the target function with volatile size to prevent optimization */
    compute_target(a, b, &sum, g_size);
    
    /* Use the result to prevent dead code elimination */
    volatile int result = sum;
    printf("Result: %d\n", result);
    
    return 0;
}
