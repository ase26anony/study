#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Prevent interprocedural optimizations
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int n, int *sum_ptr) {
    int sum = 0;
    volatile int threshold = 50; // Prevent constant propagation
    
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        // Complex conditional with multiple branches to create CFG
        if (a[i] > threshold) {
            // Nested condition to create more basic blocks
            if (b[i] > 0) {
                sum += a[i] * b[i];
            } else {
                sum += a[i] - b[i];
            }
        } else if (a[i] < -threshold) {
            // Another branch path for SIMT masking
            sum += b[i] - a[i];
        } else {
            // Default path with arithmetic operations
            sum += a[i] + b[i];
        }
        
        // Additional data-dependent operation to prevent loop simplification
        volatile int temp = a[i] % 7;
        if (temp == 0) {
            sum += 1;
        }
    }
    
    *sum_ptr = sum;
}

int main() {
    // Seed for reproducible pseudo-random values
    srand(42);
    
    // Declare and initialize arrays
    int a[SIZE], b[SIZE];
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100 - 25;  // Range: -25 to 74
        b[i] = rand() % 100 - 25;  // Range: -25 to 74
    }
    
    // Volatile to prevent compile-time optimization of loop bound
    volatile int size = SIZE;
    int sum = 0;
    
    // Call the target computation function
    compute_target(a, b, size, &sum);
    
    // Use result to prevent dead code elimination
    printf("Result sum = %d\n", sum);
    
    // Additional volatile store to ensure code generation
    volatile int dummy = sum;
    
    return 0;
}
