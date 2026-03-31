#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int val) {
    *ptr = val;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_value(int a, int b) {
    return a * 2 + b;
}

#define SIZE 1024

int main(void) {
    // Arrays with different storage classes
    int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 100;
    }
    
    // Main computational loop - high trip count encourages if-conversion
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 7 + 13) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses memory accesses with mixed types
        if (data[idx] > (int)mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] = data[idx] + 1;
            
            // 2. Function call that takes address of mod[idx] - volatile part of test
            modify_value(&mod[idx], outer % 50);
            
            // 3. Complex assignment that could alias
            int temp = compute_value(data[idx], mod[idx]);
            
            // 4. Another assignment to mod[idx] - modifies volatile part of test
            mod[idx] = data[idx] / 2 + temp % 3;
            
            // 5. Additional arithmetic to create more instructions
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
            }
            
        } else {
            // ELSE BLOCK: Different operations to ensure both paths are live
            mod[idx] = mod[idx] * 2 - 1;
            data[idx] = compute_value(mod[idx], data[idx]);
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] - mod[idx];
        
        // Complex loop with multiple conditions to encourage if-conversion analysis
        if (outer % 100 == 0) {
            // Nested if to create more complex control flow
            int idx2 = (idx + 1) % SIZE;
            if (data[idx2] < mod[idx2] * 2) {
                data[idx2] = mod[idx2] + 5;
                modify_value(&mod[idx2], data[idx2] % 100);
            }
        }
    }
    
    printf("Final accumulator: %d\n", accumulator);
    
    // Verify some results
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
