#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Function to prevent optimization and ensure target region is generated
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int size, int *sum_ptr) {
    int sum = 0;
    
    // OpenMP target region with SIMD clause - designed to trigger SIMT transformation
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: sum)
    for (int i = 0; i < size; i++) {
        // Use volatile threshold to prevent constant propagation
        volatile int threshold = 50;
        int local_threshold = threshold;
        
        // Complex conditional with data-dependent branches
        // This creates non-trivial control flow for SIMT transformation
        if (a[i] > local_threshold) {
            // Nested condition to create more basic blocks
            if (b[i] < 100) {
                sum += a[i] * b[i];
            } else {
                // Alternative computation path
                sum += a[i] + b[i];
            }
        } else if (a[i] < 20) {
            // Another conditional path
            sum += b[i] - a[i];
        } else {
            // Default path with arithmetic operations
            sum += (a[i] % 10) * (b[i] % 10);
        }
        
        // Additional arithmetic to make loop body non-trivial
        int temp = a[i] ^ b[i];
        if (temp & 1) {
            // Another conditional inside the loop
            sum += temp;
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
    
    // Use volatile for size to prevent compile-time optimization
    volatile int vsize = SIZE;
    int size = vsize;
    
    int sum = 0;
    
    // Call the target computation function
    compute_target(a, b, size, &sum);
    
    // Use result to prevent dead code elimination
    printf("Result: %d\n", sum);
    
    // Additional volatile store to ensure code isn't optimized away
    volatile int check = sum;
    
    return 0;
}
