#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Function to prevent interprocedural optimizations
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int n, int *sum_ptr) {
    int sum = 0;
    
    // Complex OpenMP target region with SIMD clause
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) \
        map(to: a[0:n], b[0:n]) \
        map(tofrom: sum) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < n; i++) {
        // Volatile threshold to prevent constant propagation
        volatile int threshold = 50;
        int local_threshold = threshold;
        
        // Data-dependent conditional branch - creates control flow
        if (a[i] > local_threshold) {
            // Nested condition to create more complex CFG
            if (b[i] < 100) {
                sum += a[i] * b[i];
            } else {
                // Alternative computation path
                sum += a[i] + b[i];
            }
        } else {
            // Another branch with different computation
            if (a[i] < 20) {
                sum -= b[i];
            } else {
                // Complex arithmetic to prevent simplification
                sum += (a[i] * 2) - (b[i] / 3);
            }
        }
        
        // Additional conditional to create more basic blocks
        if ((a[i] + b[i]) % 7 == 0) {
            sum += 1;
        }
    }
    
    *sum_ptr = sum;
}

int main() {
    // Seed random number generator
    srand(42);
    
    // Declare and initialize arrays
    int a[SIZE], b[SIZE];
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    // Volatile size to prevent compile-time optimization
    volatile int size = SIZE;
    
    // Reduction variable
    int sum = 0;
    
    // Call the target computation function
    compute_target(a, b, size, &sum);
    
    // Use result to prevent dead code elimination
    volatile int result = sum;
    printf("Result: %d\n", result);
    
    return 0;
}
