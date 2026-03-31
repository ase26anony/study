#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define SIZE 1024

// Function to prevent interprocedural optimization
static void __attribute__((noinline, noipa))
compute_target(int *a, int *b, int n, int *sum_ptr) {
    int sum = 0;
    
    // Complex OpenMP target region with SIMD clause
    #pragma omp target teams distribute parallel for simd \
                reduction(+:sum) \
                map(to: a[0:n], b[0:n]) \
                map(tofrom: sum)
    for (int i = 0; i < n; i++) {
        // Volatile threshold to prevent constant propagation
        volatile int threshold = 50;
        int local_thresh = threshold;
        
        // Data-dependent conditional with arithmetic operations
        // Creates non-trivial control flow graph
        if (a[i] > local_thresh) {
            // Complex arithmetic operation
            int temp = a[i] * b[i];
            
            // Nested conditional for additional complexity
            if (temp > 1000) {
                sum += temp * 2;
            } else {
                sum += temp;
            }
        } else if (a[i] < 20) {
            // Another conditional branch for more complex CFG
            sum -= b[i] / 2;
        }
        
        // Additional operation to create more basic blocks
        int mod_result = a[i] % 7;
        if (mod_result == 0) {
            sum += 1;
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
    
    // Volatile size to prevent compile-time optimization
    volatile int size = SIZE;
    int actual_size = size;
    
    int sum = 0;
    
    // Call the target computation function
    compute_target(a, b, actual_size, &sum);
    
    // Use result to prevent dead code elimination
    printf("Result: %d\n", sum);
    
    // Additional volatile store to ensure code generation
    volatile int dummy = sum;
    
    return 0;
}
