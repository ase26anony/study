#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Prevent interprocedural optimizations
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, volatile int size, int *sum_ptr) {
    int sum = 0;
    
    // Complex OpenMP target region with SIMD clause
    #pragma omp target teams distribute parallel for simd \
        reduction(+:sum) \
        map(to: a[0:size], b[0:size]) \
        map(tofrom: sum)
    for (int i = 0; i < size; i++) {
        // Volatile threshold to prevent constant propagation
        volatile int threshold = 50;
        int local_threshold = threshold;
        
        // Data-dependent conditional branch - creates non-trivial CFG
        if (a[i] > local_threshold) {
            // Nested conditional to increase CFG complexity
            if (b[i] < 100) {
                // Arithmetic operations amenable to vectorization
                int temp = a[i] * b[i];
                
                // Additional conditional to create masking requirements
                if (temp > 0) {
                    sum += temp;
                } else {
                    sum += a[i] + b[i];
                }
            } else {
                // Alternative computation path
                sum += a[i] - b[i];
            }
        } else {
            // Another branch for the SIMT transformation to handle
            if (a[i] < 20) {
                sum += b[i] * 2;
            }
        }
        
        // Additional arithmetic to create more vectorizable operations
        int dummy = a[i] + b[i];
        if (dummy % 2 == 0) {
            // Empty side effect to maintain complexity
            dummy = dummy * 2;
        }
    }
    
    *sum_ptr = sum;
}

int main() {
    // Seed for reproducible results
    srand(42);
    
    // Use volatile size to prevent compile-time optimization
    volatile int size = 1024;
    int actual_size = size;
    
    // Allocate and initialize arrays with pseudo-random data
    int *a = (int*)malloc(actual_size * sizeof(int));
    int *b = (int*)malloc(actual_size * sizeof(int));
    
    for (int i = 0; i < actual_size; i++) {
        a[i] = rand() % 100;      // Values 0-99
        b[i] = rand() % 200;      // Values 0-199
    }
    
    int sum = 0;
    
    // Call the target computation function
    compute_target(a, b, size, &sum);
    
    // Use the result to prevent dead code elimination
    printf("Result sum = %d\n", sum);
    
    // Additional volatile store to ensure code isn't optimized away
    volatile int check = sum;
    
    free(a);
    free(b);
    
    return 0;
}
