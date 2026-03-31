#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Function to prevent optimization and ensure target region is generated
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int n, int *sum_ptr) {
    int sum = 0;
    volatile int threshold = 50;  // Prevent constant propagation
    
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) \
                map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        // Complex conditional with data-dependent branches
        if (a[i] > threshold) {
            // Nested condition to create more complex CFG
            if (b[i] < 100) {
                sum += a[i] * b[i];
            } else {
                sum += a[i] + b[i];
            }
        } else if (a[i] < 20) {
            // Another branch to ensure non-trivial control flow
            sum -= b[i];
        } else {
            // Default path with arithmetic operations
            sum += (a[i] % 10) * (b[i] % 10);
        }
        
        // Additional operation to prevent loop simplification
        volatile int dummy = i;  // Prevent dead code elimination
    }
    
    *sum_ptr = sum;
}

int main() {
    // Seed random number generator for reproducible results
    srand(42);
    
    // Declare and initialize arrays
    int a[SIZE], b[SIZE];
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    // Use volatile to prevent compile-time optimization of loop bounds
    volatile int size = SIZE;
    int sum = 0;
    
    // Call the target computation function
    compute_target(a, b, size, &sum);
    
    // Use the result to prevent dead code elimination
    printf("Result: %d\n", sum);
    
    return 0;
}
