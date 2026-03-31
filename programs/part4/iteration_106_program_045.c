#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr) {
    *ptr += 1;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int x, int y) {
    return (x + y) % 100;
}

int main(void) {
    // Arrays with different storage classes
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    // Main loop - high trip count encourages if-conversion
    for (int outer = 0; outer < ITERATIONS; ++outer) {
        // Complex index calculation
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses memory accesses with volatile
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test_expr
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] -= 2;
            
            // 2. Function call with address of volatile mod[idx]
            // This creates aliasing concerns
            modify_value(&mod[idx]);
            
            // 3. Assignment that could alias with condition
            int temp = data[idx];
            mod[idx] = temp / 2 + compute_threshold(temp, idx);
            
            // 4. More arithmetic operations
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
            }
            
            // 5. Pointer arithmetic that might confuse analysis
            int* data_ptr = &data[idx];
            *data_ptr = (*data_ptr % 100) + 1;
            
            // Final use to prevent elimination
            accumulator += data[idx] * 2;
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = data[idx] + compute_threshold(data[idx], mod[idx]);
            accumulator -= data[idx];
        }
        
        // Additional computation to prevent dead code elimination
        if (outer % 1000 == 0) {
            accumulator += data[idx % SIZE] + mod[(idx + 1) % SIZE];
        }
    }
    
    // Use results to prevent optimization
    printf("Result: %d\n", accumulator);
    
    // Verify array state
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += data[i];
    }
    printf("Array sum: %d\n", sum);
    
    return 0;
}
