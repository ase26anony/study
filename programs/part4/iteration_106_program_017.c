#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int value) {
    *ptr = value;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int a, int b) {
    return (a + b) / 3;
}

int main(void) {
    // Arrays with different storage classes
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    int temp[SIZE];
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
        temp[i] = 0;
    }
    
    volatile int accumulator = 0;
    int idx = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < ITERATIONS; outer++) {
        // Calculate index with wrap-around
        idx = (outer * 7) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Complex test expression involving memory access and arithmetic
        if (data[idx] > mod[idx] + compute_threshold(outer, idx)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] = data[idx] - 1;
            
            // 2. Function call that takes address of mod[idx] - volatile part of test
            modify_value(&mod[idx], data[idx] * 2);
            
            // 3. Arithmetic that creates potential aliasing
            int* alias_ptr = &data[idx];
            *alias_ptr = *alias_ptr + mod[idx] / 4;
            
            // 4. More complex operations
            for (int j = 0; j < 3; j++) {
                temp[(idx + j) % SIZE] = data[idx] + j;
            }
            
            // 5. Final assignment that could affect test expression
            mod[idx] = data[idx] / 2 + outer % 10;
            
        } else {
            // ELSE BLOCK: Different operations
            data[idx] = data[idx] + 2;
            mod[idx] = mod[idx] - 1;
            
            // Additional computation to keep block non-trivial
            int local_sum = 0;
            for (int k = 0; k < 2; k++) {
                local_sum += data[(idx + k) % SIZE];
            }
            temp[idx] = local_sum;
        }
        
        // Additional operations to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 1000 == 0) {
            // Mix data types in computation
            double ratio = (double)data[idx] / (mod[idx] + 1.0);
            accumulator += (int)(ratio * 100);
        }
    }
    
    printf("Result: %d\n", accumulator);
    
    // Verify some results
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += data[i] + temp[i];
    }
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
