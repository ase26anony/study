#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the OpenMP construct */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int n, int *sum_ptr) {
    int sum = 0;
    
    /* Use volatile threshold to prevent constant propagation */
    volatile int threshold = 50;
    int local_threshold = threshold;
    
    /* 
     * Combined target teams distribute parallel for simd construct
     * This should trigger SIMT transformation in omp-low.cc
     */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        /* 
         * Complex conditional with data-dependent branches
         * Creates non-trivial control flow for SIMT transformation
         */
        if (a[i] > local_threshold) {
            /* First conditional branch with arithmetic */
            sum += a[i] * b[i];
            
            /* Additional conditional to create more basic blocks */
            if (b[i] < local_threshold * 2) {
                /* Nested conditional with different operation */
                sum -= a[i] / 2;
            } else {
                /* Alternative path with bitwise operation */
                sum ^= (a[i] << 1);
            }
        } else if (a[i] < local_threshold / 2) {
            /* Second conditional branch path */
            sum += b[i] - a[i];
            
            /* Another nested conditional */
            if ((a[i] + b[i]) % 3 == 0) {
                sum += 1;
            }
        } else {
            /* Third path for middle values */
            sum += (a[i] * 3) / (b[i] + 1);
        }
        
        /* Additional operation to prevent loop simplification */
        sum += (i % 8) ? 0 : 1;
    }
    
    *sum_ptr = sum;
}

int main() {
    /* Seed random number generator */
    srand(42);
    
    /* Declare and initialize arrays */
    int a[SIZE], b[SIZE];
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Use volatile size to prevent compile-time optimization */
    volatile int size = SIZE;
    int sum = 0;
    
    /* Call the target computation function */
    compute_target(a, b, size, &sum);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    /* Additional volatile store to ensure code generation */
    volatile int dummy = sum;
    
    return 0;
}
