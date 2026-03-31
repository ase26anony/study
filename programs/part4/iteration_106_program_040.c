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
int compute_threshold(int base) {
    return (base * 3) / 2;
}

int main() {
    // Arrays with different storage classes
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = (i * 2) % 100;
    }
    
    // Main computational loop - designed to trigger if-conversion
    for (int outer = 0; outer < ITERATIONS; outer++) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 31) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and volatile variable
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test_expr
            // 1. Modify data[idx] - could alias with test_expr
            int temp = data[idx];
            temp += 2;
            
            // 2. Function call with address of volatile variable
            // This creates complex RTL patterns
            modify_value(&mod[idx], temp / 3);
            
            // 3. Arithmetic that might be seen as modifying test_expr
            // Use pointer arithmetic to create potential aliasing
            int* data_ptr = &data[idx];
            *data_ptr = compute_threshold(*data_ptr);
            
            // 4. Another assignment that could affect test_expr
            mod[idx] = data[idx] * 2 - 15;
            
            // 5. Additional computation to extend basic block
            for (int j = 0; j < 3; j++) {
                mod[idx] += j;
            }
            
            // Final modification in then block
            data[idx] = mod[idx] + 1;
        } else {
            // ELSE BLOCK: Different operations
            mod[idx] = data[idx] - 5;
            data[idx] = mod[idx] * 2;
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 100 == 0) {
            // Nested condition to create more control flow
            int idx2 = (idx + 1) % SIZE;
            if (data[idx2] < mod[idx]) {
                data[idx2] = mod[idx] + data[idx];
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
