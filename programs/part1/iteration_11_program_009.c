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
        // Complex conditional with multiple data-dependent branches
        // to create non-trivial control flow
        if (a[i] > threshold) {
            // Nested condition to create more basic blocks
            if (b[i] > threshold / 2) {
                sum += a[i] * b[i];
            } else {
                sum += a[i] + b[i];
            }
        } else if (a[i] < threshold / 3) {
            // Another branch path
            sum += b[i] - a[i];
        } else {
            // Default path with arithmetic operations
            sum += (a[i] % 10) * (b[i] % 10);
        }
        
        // Additional arithmetic to make loop body non-trivial
        int temp = a[i] ^ b[i];
        if (temp & 1) {
            sum += temp;
        }
    }
    
    *sum_ptr = sum;
}

int main() {
    // Seed for reproducible results
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
    
    // Call the target function
    compute_target(a, b, size, &sum);
    
    // Use result to prevent dead code elimination
    printf("Result: %d\n", sum);
    
    // Additional volatile store as backup
    volatile int result_check = sum;
    
    return 0;
}
