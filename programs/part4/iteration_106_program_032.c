#include <stdio.h>
#include <stdlib.h>

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

int main() {
    // Arrays with different storage classes
    int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 30);
    }
    
    // Main computational loop - designed to trigger if-conversion analysis
    for (int outer = 0; outer < ITERATIONS; outer++) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and arithmetic
        if (data[idx] > mod[idx] + compute_threshold(idx, outer)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] = data[idx] - 1;
            
            // 2. Function call that might alias with test expression
            modify_value(&mod[idx], data[idx] * 2);
            
            // 3. Arithmetic that could be seen as modifying the test expression
            int temp = data[idx] + mod[idx];
            data[idx] = temp / 2;
            
            // 4. Another operation that uses the same memory
            mod[idx] = data[idx] + (idx % 10);
            
            // 5. Complex sequence to create multiple basic blocks
            for (int j = 0; j < 3; j++) {
                data[idx] += j;
                if (j % 2 == 0) {
                    mod[idx] -= 1;
                }
            }
            
            // Final assignment in then block
            accumulator += data[idx] * 2;
            
        } else {
            // ELSE BLOCK: Different operations to keep both paths live
            mod[idx] = mod[idx] + data[idx];
            data[idx] = data[idx] * 3 / 2;
            accumulator -= mod[idx];
        }
        
        // Additional computation to prevent dead code elimination
        if (outer % 100 == 0) {
            // Complex condition with side effects
            int temp_idx = (idx + 1) % SIZE;
            if (data[temp_idx] < mod[temp_idx]) {
                data[temp_idx] = mod[temp_idx] - data[temp_idx];
                modify_value(&mod[temp_idx], data[temp_idx]);
            }
        }
        
        // Use volatile to prevent optimization
        if (accumulator > 1000000) {
            accumulator = accumulator % 1000000;
        }
    }
    
    // Final result
    printf("Result: %d\n", accumulator);
    
    // Additional verification to prevent optimization
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += data[i] + mod[i];
    }
    printf("Checksum: %d\n", sum);
    
    return 0;
}
