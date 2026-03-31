#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Function to prevent interprocedural optimizations
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int size, int *sum_ptr) {
    int sum = 0;
    volatile int threshold = 50; // Prevent constant propagation
    
    // Complex OpenMP target region with SIMD clause
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) \
        map(to: a[0:size], b[0:size]) \
        map(tofrom: sum) \
        num_teams(4) thread_limit(128)
    for (int i = 0; i < size; i++) {
        // Data-dependent conditional with multiple branches
        // to create non-trivial control flow
        if (a[i] > threshold) {
            // Nested condition to create more basic blocks
            if (b[i] != 0) {
                sum += a[i] * b[i];
            } else {
                sum += a[i];
            }
        } else if (a[i] < -threshold) {
            // Another branch path for negative values
            sum -= b[i];
        } else {
            // Default path with arithmetic operations
            int temp = a[i] + b[i];
            if (temp > 0) {
                sum += temp;
            }
        }
        
        // Additional computation to prevent loop simplification
        volatile int dummy = i; // Prevent dead code elimination
        (void)dummy;
    }
    
    *sum_ptr = sum;
}

int main() {
    // Initialize with deterministic random values
    srand(42);
    
    int a[SIZE], b[SIZE];
    volatile int size = SIZE; // Runtime-determined size
    
    // Initialize arrays with pseudo-random data
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 200 - 100; // Values between -100 and 99
        b[i] = rand() % 200 - 100;
    }
    
    int sum = 0;
    
    // Call the target computation function
    compute_target(a, b, size, &sum);
    
    // Use the result to prevent dead code elimination
    printf("Result: %d\n", sum);
    
    // Additional volatile store to ensure code isn't optimized away
    volatile int result_check = sum;
    (void)result_check;
    
    return 0;
}
