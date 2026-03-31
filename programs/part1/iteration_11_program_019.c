#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* 
 * This function must not be inlined or interprocedural optimized
 * to preserve the OpenMP target construct structure
 */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int size, int *sum_result)
{
    int sum = 0;
    
    /* 
     * Complex OpenMP target region with SIMD clause that should trigger
     * the SIMT transformation in omp-low.cc lines 2941-2975
     * 
     * The combination of:
     * - target teams distribute parallel for simd
     * - reduction with conditional inside loop
     * - data mapping
     * should generate IFN_GOMP_USE_SIMT call
     */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: sum)
    for (int i = 0; i < size; i++) {
        /* 
         * Complex conditional reduction to create non-trivial CFG
         * that requires SIMT masking transformation
         */
        volatile int threshold = 50;  /* volatile to prevent constant prop */
        
        /* First conditional branch - creates basic block structure */
        if (a[i] > threshold) {
            /* Nested conditional to increase CFG complexity */
            if (b[i] != 0) {
                /* Arithmetic operations amenable to vectorization */
                int temp = a[i] * b[i];
                
                /* Another conditional to create more basic blocks */
                if (temp > 1000) {
                    sum += temp / 2;
                } else {
                    sum += temp;
                }
            }
        } else if (a[i] > threshold / 2) {
            /* Alternative path in CFG */
            sum += a[i] + b[i];
        }
        
        /* Additional operation to prevent loop simplification */
        sum += (i % 2 == 0) ? 1 : -1;
    }
    
    *sum_result = sum;
}

int main(void)
{
    /* Use constant seed for reproducibility */
    srand(42);
    
    /* Array size - volatile to prevent compile-time optimization */
    volatile int size = 1024;
    
    /* Allocate and initialize arrays with pseudo-random data */
    int *a = (int *)malloc(size * sizeof(int));
    int *b = (int *)malloc(size * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with values that create varied control flow */
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;      /* Values 0-99 */
        b[i] = rand() % 200 - 50; /* Values -50 to 149 */
    }
    
    int sum = 0;
    
    /* Call the target function - this should trigger the SIMT transformation */
    compute_target(a, b, size, &sum);
    
    /* Use result to prevent dead code elimination */
    printf("Result sum = %d\n", sum);
    
    /* Additional volatile store to ensure code isn't optimized away */
    volatile int check = sum;
    (void)check;  /* Suppress unused variable warning */
    
    free(a);
    free(b);
    
    return 0;
}
