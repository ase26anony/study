#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Prevent interprocedural optimizations
static void compute_target(int *a, int *b, int size, int *sum) 
    __attribute__((noinline, noipa));

static void compute_target(int *a, int *b, int size, int *sum)
{
    int local_sum = 0;
    volatile int threshold = 50; // Prevent constant propagation
    
    // Complex loop with data-dependent conditionals - triggers SIMT transformation
    #pragma omp target teams distribute parallel for simd \
                reduction(+:local_sum) \
                map(to: a[0:size], b[0:size]) \
                map(tofrom: local_sum)
    for (int i = 0; i < size; i++) {
        // Multiple data-dependent conditionals create complex control flow
        if (a[i] > threshold) {
            local_sum += a[i] * b[i];
            
            // Additional conditional to create more basic blocks
            if (b[i] < threshold / 2) {
                local_sum -= a[i]; // Another operation
            } else {
                local_sum += b[i]; // Different path
            }
        } else if (a[i] < threshold / 3) {
            // Alternative conditional path
            local_sum += b[i] - a[i];
        }
        
        // More arithmetic to prevent loop simplification
        int temp = a[i] ^ b[i];
        if (temp & 0x1) {
            local_sum += 1;
        }
    }
    
    *sum = local_sum;
}

int main(void)
{
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
    volatile int result = sum; // Force memory store
    printf("Result: %d\n", sum);
    
    return 0;
}
