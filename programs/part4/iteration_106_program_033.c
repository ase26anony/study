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
    return (a + b) / 2;
}

int main(void) {
    // Arrays with different storage classes
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    // Main computational loop - high trip count encourages if-conversion
    for (int outer = 0; outer < ITERATIONS; outer++) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory accesses to both arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] = data[idx] + 1;
            
            // 2. Function call that takes address of mod[idx] - could alias
            modify_value(&mod[idx], data[idx] / 2);
            
            // 3. Complex assignment that might be seen as modifying test expression
            int temp = compute_threshold(data[idx], mod[idx]);
            mod[idx] = temp * 2 - data[idx];
            
            // 4. Additional arithmetic to create more RTL instructions
            for (int j = 0; j < 3; j++) {
                data[idx] += j;
            }
            
            // 5. Pointer arithmetic that could confuse alias analysis
            int* data_ptr = &data[idx];
            volatile int* mod_ptr = &mod[idx];
            *data_ptr = *data_ptr - (*mod_ptr % 10);
            
        } else {
            // ELSE BLOCK: Different operations to ensure both paths are live
            mod[idx] = mod[idx] * 2;
            data[idx] = data[idx] - 1;
            
            // Additional computation to match complexity
            if (mod[idx] < 0) {
                mod[idx] = 0;
            }
        }
        
        // Prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Additional loop to create more basic blocks
        for (int k = 0; k < 2; k++) {
            if (accumulator > 1000000) {
                accumulator /= 2;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
