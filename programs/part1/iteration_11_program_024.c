#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Function to prevent interprocedural optimization
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int size, int *sum) {
    volatile int threshold = 50; // Prevent constant propagation
    
    #pragma omp target teams distribute parallel for simd \
                reduction(+:*sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: *sum)
    for (int i = 0; i < size; i++) {
        // Complex conditional with data-dependent branches
        // This creates non-trivial control flow for SIMT transformation
        if (a[i] > threshold) {
            // Nested conditional to create more basic blocks
            if (b[i] < 100) {
                *sum += a[i] * b[i];
            } else {
                // Alternative computation path
                *sum += a[i] - b[i];
            }
        } else {
            // Another branch for the else case
            if (a[i] < 20) {
                *sum += b[i] / 2;
            }
        }
        
        // Additional arithmetic to create more complex CFG
        int temp = a[i] + b[i];
        if (temp % 3 == 0) {
            *sum += temp;
        }
    }
}

int main() {
    // Seed random number generator
    srand(42);
    
    // Declare arrays
    int a[SIZE], b[SIZE];
    
    // Initialize with pseudo-random data
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    // Use volatile to prevent compile-time optimization of size
    volatile int size = SIZE;
    
    // Reduction variable
    int sum = 0;
    
    // Call the target computation function
    compute_target(a, b, (int)size, &sum);
    
    // Use the result to prevent dead code elimination
    printf("Result sum = %d\n", sum);
    
    // Additional volatile store to ensure code isn't optimized away
    volatile int dummy = sum;
    
    return 0;
}
