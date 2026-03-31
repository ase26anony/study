#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Function to prevent interprocedural optimization
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
        }
        
        // Additional conditional to create more complex CFG
        if (b[i] < threshold / 2) {
            // Nested condition for more basic blocks
            if (a[i] % 2 == 0) {
                sum -= b[i];
            } else {
                sum += a[i];
            }
        }
        
        // Another conditional path
        if (a[i] * b[i] > 1000) {
            sum += 2;
        }
    }
    
    *sum_ptr = sum;
}

int main() {
    // Initialize with deterministic random values
    srand(42);
    
    int a[SIZE], b[SIZE];
    volatile int size = SIZE; // Prevent compile-time optimization
    
    // Initialize arrays with pseudo-random data
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
    }
    
    int sum = 0;
    
    // Call the target function
    compute_target(a, b, size, &sum);
    
    // Use result to prevent dead code elimination
    printf("Result sum = %d\n", sum);
    
    // Additional volatile store to ensure code isn't optimized away
    volatile int dummy = sum;
    
    return 0;
}
