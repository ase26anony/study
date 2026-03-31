#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Prevent interprocedural optimizations
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int n, int *sum_ptr) {
    int sum = 0;
    volatile int threshold = 50; // Prevent constant propagation
    
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) \
                map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        // Complex conditional with data-dependent branches
        if (a[i] > threshold) {
            sum += a[i] * b[i];
        } else if (a[i] > threshold / 2) {
            // Additional branch to create non-trivial CFG
            sum += (a[i] + b[i]) / 2;
        } else {
            // Third branch path for more complex control flow
            sum += a[i] - b[i];
        }
        
        // Additional arithmetic to prevent loop simplification
        int temp = a[i] ^ b[i];
        if (temp & 1) {
            sum += 1;
        }
    }
    
    *sum_ptr = sum;
}

int main() {
    // Initialize with deterministic random values
    srand(42);
    
    int a[SIZE], b[SIZE];
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    volatile int size = SIZE; // Prevent compile-time optimization
    int sum = 0;
    
    // Call the target function
    compute_target(a, b, size, &sum);
    
    // Use result to prevent dead code elimination
    printf("Result: %d\n", sum);
    
    return 0;
}
