#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the target region */
static void compute_target(int *a, int *b, int size, int *sum) 
    __attribute__((noinline, noipa));

static void compute_target(int *a, int *b, int size, int *sum) {
    int local_sum = 0;
    
    /* 
     * Complex target region with SIMD clause to trigger SIMT transformation.
     * The volatile threshold prevents constant propagation.
     */
    volatile int threshold = 50;
    int thresh = threshold;  /* Use volatile value to prevent optimization */
    
    #pragma omp target teams distribute parallel for simd \
                reduction(+:local_sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: local_sum)
    for (int i = 0; i < size; i++) {
        /* 
         * Complex conditional reduction with multiple data-dependent branches
         * to create non-trivial control flow for SIMT transformation
         */
        if (a[i] > thresh) {
            /* First conditional branch */
            local_sum += a[i] * b[i];
            
            /* Additional conditional to create more basic blocks */
            if (b[i] < thresh) {
                /* Nested conditional for complex CFG */
                local_sum -= a[i];
            } else if (b[i] > thresh * 2) {
                /* Another branch path */
                local_sum += b[i];
            }
        } else if (a[i] < thresh / 2) {
            /* Alternative conditional path */
            local_sum += b[i] - a[i];
            
            /* More complex arithmetic to prevent simplification */
            if ((a[i] + b[i]) % 3 == 0) {
                local_sum += 1;
            }
        }
        
        /* Additional operation to ensure loop body isn't trivial */
        local_sum += (i % 2 == 0) ? 0 : 1;
    }
    
    *sum = local_sum;
}

int main() {
    /* Initialize with deterministic random values */
    srand(42);
    
    int a[SIZE], b[SIZE];
    volatile int size = SIZE;  /* Volatile to prevent constant propagation */
    int sum = 0;
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result sum = %d\n", sum);
    
    /* Additional use to ensure code isn't optimized away */
    volatile int check = sum;
    
    return 0;
}
