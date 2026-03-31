#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

/* Prevent interprocedural optimizations from removing the target region */
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int n, int *sum_ptr) {
    int sum = 0;
    
    /* Target region with SIMD clause - should trigger SIMT transformation */
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        /* Complex conditional with data-dependent branches */
        volatile int threshold = 50; /* Prevent constant propagation */
        
        /* Multiple conditionals to create non-trivial control flow */
        if (a[i] > threshold) {
            sum += a[i] * b[i];
        }
        
        /* Additional conditional to increase complexity */
        if (b[i] < threshold / 2) {
            sum -= a[i];
        }
        
        /* Nested conditional for more complex CFG */
        if (a[i] % 2 == 0) {
            if (b[i] % 3 == 0) {
                sum += 1;
            }
        }
    }
    
    *sum_ptr = sum;
}

int main() {
    int a[SIZE], b[SIZE];
    volatile int size = SIZE; /* Prevent loop unrolling/optimization */
    int sum = 0;
    
    /* Seed for reproducible results */
    srand(42);
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    /* Call target function */
    compute_target(a, b, size, &sum);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    /* Additional volatile store to ensure code isn't optimized away */
    volatile int check = sum;
    
    return 0;
}
