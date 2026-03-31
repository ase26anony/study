/* This program is designed to trigger the SIMT transformation in GCC's omp-low.cc
   Specifically targeting lines 2941-2975 which handle IFN_GOMP_USE_SIMT generation */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int g_size = 1024;

/* Function containing the target region - marked to prevent optimization */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int size, int *sum)
{
    int local_sum = 0;
    volatile int threshold = 50; /* Volatile to prevent constant propagation */
    
    /* Combined target teams distribute parallel for simd construct
       This is the key construct that should trigger SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:local_sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: local_sum) \
                num_teams(4) thread_limit(128)
    for (int i = 0; i < size; i++) {
        /* Complex conditional with data-dependent branches
           Creates non-trivial control flow for SIMT transformation */
        if (a[i] > threshold) {
            local_sum += a[i] * b[i];
            /* Additional conditional to create more basic blocks */
            if (b[i] < threshold / 2) {
                local_sum += a[i];
            }
        } else if (a[i] > threshold / 2) {
            /* Another conditional path */
            local_sum += b[i];
        }
        
        /* Additional arithmetic to make loop body non-trivial */
        int temp = a[i] - b[i];
        if (temp > 0) {
            local_sum += temp;
        }
    }
    
    *sum = local_sum;
}

int main(void)
{
    /* Seed random number generator for reproducible results */
    srand(42);
    
    const int size = g_size; /* Use volatile size indirectly */
    int *a = (int *)malloc(size * sizeof(int));
    int *b = (int *)malloc(size * sizeof(int));
    
    if (!a || !b) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    int sum = 0;
    
    /* Call the target function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result sum = %d\n", sum);
    
    /* Additional volatile store to ensure code isn't optimized away */
    volatile int check = sum;
    (void)check;
    
    free(a);
    free(b);
    
    return 0;
}
