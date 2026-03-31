#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Prevent interprocedural optimizations
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int size, int *sum_result) {
    int sum = 0;
    volatile int threshold = 50; // Prevent constant propagation
    
    // Complex OpenMP target region with SIMD
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) \
        map(to: a[0:size], b[0:size]) \
        map(tofrom: sum) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < size; i++) {
        // Data-dependent conditional branch - creates control flow
        if (a[i] > threshold) {
            // Nested condition to create more complex CFG
            if (b[i] < 100) {
                sum += a[i] * b[i];
            } else {
                sum += a[i] + b[i];
            }
        } else if (a[i] < threshold / 2) {
            // Another conditional path
            sum += b[i] - a[i];
        }
        
        // Additional arithmetic to prevent loop simplification
        volatile int temp = a[i] ^ b[i];
        if (temp & 1) {
            sum += 1;
        }
    }
    
    *sum_result = sum;
}

int main() {
    // Initialize with deterministic random values
    srand(42);
    
    int a[SIZE];
    int b[SIZE];
    
    // Initialize arrays with pseudo-random data
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    // Use volatile to prevent compile-time optimization
    volatile int size = SIZE;
    int sum = 0;
    
    // Call the target computation function
    compute_target(a, b, (int)size, &sum);
    
    // Use result to prevent dead code elimination
    printf("Result: %d\n", sum);
    
    // Additional use to ensure code isn't optimized away
    volatile int dummy = sum;
    
    return 0;
}
