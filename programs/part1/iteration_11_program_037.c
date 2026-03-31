#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the OpenMP construct */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int n, int *sum_ptr) {
    int sum = 0;
    
    /* 
     * Complex OpenMP target region with SIMD clause.
     * The volatile threshold prevents constant propagation.
     * The conditional reduction creates data-dependent control flow.
     */
    volatile int threshold = 50;
    
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) \
                map(tofrom: sum)
    for (int i = 0; i < n; ++i) {
        /* 
         * Data-dependent conditional branch - creates non-trivial CFG
         * This should trigger SIMT transformation for masking
         */
        if (a[i] > threshold) {
            sum += a[i] * b[i];
            
            /* Additional conditional to create more complex CFG */
            if (b[i] < threshold / 2) {
                sum -= a[i];  /* Nested conditional operation */
            }
        } else {
            /* Alternative path to ensure both branches exist */
            if (a[i] < threshold / 3) {
                sum += b[i];  /* Different computation for else branch */
            }
        }
        
        /* Additional arithmetic to make loop body non-trivial */
        int temp = a[i] + b[i];
        if (temp % 2 == 0) {
            /* Another conditional to increase CFG complexity */
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
    for (int i = 0; i < SIZE; ++i) {
        a[i] = rand() % 100;  /* Values 0-99 */
        b[i] = rand() % 100;
    }
    
    /* Use volatile to prevent compile-time optimization of loop bound */
    volatile int size = SIZE;
    int sum = 0;
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use the result to prevent dead code elimination */
    printf("Result sum: %d\n", sum);
    
    /* Additional volatile store to ensure code isn't optimized away */
    volatile int check = sum;
    (void)check;  /* Prevent unused variable warning */
    
    return 0;
}
