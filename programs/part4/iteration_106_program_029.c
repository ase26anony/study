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
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    // Main computational loop - high trip count encourages if-conversion
    for (int outer = 0; outer < ITERATIONS; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access to volatile and non-volatile arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            // 1. Modify data[idx] - could affect test expression if aliased
            int temp = data[idx];
            temp += 2;
            
            // 2. Function call with address of volatile array element
            // This creates complex RTL that might be analyzed by modified_in_p
            modify_value(&mod[idx], temp / 2);
            
            // 3. Additional arithmetic that could be seen as modifying
            // the memory locations used in the test expression
            data[idx] = temp - 1;
            
            // 4. Another operation on the test expression components
            int threshold = compute_threshold(data[idx], mod[idx]);
            
            // 5. Final assignment that definitely modifies mod[idx]
            // which is part of the original test expression
            mod[idx] = threshold + (data[idx] % 10);
            
            // Additional statements to create more basic blocks
            if (data[idx] % 3 == 0) {
                mod[idx] *= 2;
            }
            
        } else {
            // ELSE BLOCK: Different operations to ensure both paths are live
            mod[idx] = data[idx] * 3;
            data[idx] = mod[idx] / 4;
            
            // Nested condition in else to create more complex CFG
            if (mod[idx] < 100) {
                data[idx] += 5;
            }
        }
        
        // Additional computation using results to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 100 == 0) {
            // This creates additional control flow
            int next_idx = (idx + 1) % SIZE;
            if (data[next_idx] > mod[next_idx]) {
                mod[next_idx] = data[next_idx];
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
