/* This program is designed to trigger the SIMT transformation in GCC's omp-low.cc
   Specifically targeting lines 2941-2975 which handle IFN_GOMP_USE_SIMT */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Prevent interprocedural optimizations from removing the OpenMP construct */
static void compute_target(int *a, int *b, int size, int *sum) 
    __attribute__((noinline, noipa));

static void compute_target(int *a, int *b, int size, int *sum) {
    /* Use volatile to prevent constant propagation of threshold */
    volatile int threshold = 50;
    int local_threshold = threshold;
    
    /* Complex loop with data-dependent conditionals to create non-trivial CFG
       The SIMT transformation will create conditional SIMT version of this loop */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:*sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: *sum)
    for (int i = 0; i < size; i++) {
        /* Multiple data-dependent conditionals to create masking requirements */
        if (a[i] > local_threshold) {
            /* Nested conditional to increase control flow complexity */
            if (b[i] < 100) {
                *sum += a[i] * b[i];
            } else {
                /* Alternative computation path */
                *sum += a[i] + b[i];
            }
        } else if (a[i] > (local_threshold / 2)) {
            /* Another conditional path for partial vectorization */
            *sum += a[i] - b[i];
        }
        
        /* Additional arithmetic operations to create more basic blocks */
        int temp = a[i] * 2;
        if (temp > b[i]) {
            /* This creates another conditional branch in the loop body */
            *sum += temp;
        }
    }
}

int main() {
    /* Use constant seed for reproducibility */
    srand(42);
    
    /* Volatile size to prevent compile-time loop unrolling/simplification */
    volatile int vsize = 1024;
    const int size = vsize;
    
    /* Arrays with pseudo-random data */
    int *a = (int *)malloc(size * sizeof(int));
    int *b = (int *)malloc(size * sizeof(int));
    
    /* Initialize with random values to ensure data-dependent branches */
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;    /* Values 0-99 */
        b[i] = rand() % 200;    /* Values 0-199 */
    }
    
    int sum = 0;
    
    /* Call the target function containing the OpenMP region */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    volatile int result = sum;
    printf("Result: %d\n", result);
    
    /* Additional printf to ensure code isn't optimized away */
    printf("Array elements: %d, %d\n", a[0], b[0]);
    
    free(a);
    free(b);
    
    return 0;
}
