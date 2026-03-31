#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int delta) {
    *ptr += delta;
}

// Another opaque function that could alias
__attribute__((noinline, noclone))
int* get_pointer(int* base, int idx) {
    return &base[idx];
}

int main(void) {
    // Arrays with different storage classes
    int data[1024];
    volatile int mod[1024];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < 1024; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % 1024;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access to volatile and non-volatile arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] = data[idx] + 1;
            
            // 2. Function call that takes address of mod[idx] - volatile part of test
            modify_value(&mod[idx], -1);
            
            // 3. Complex assignment that could alias
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = *alias_ptr / 2;
            
            // 4. Another operation on mod[idx]
            mod[idx] = data[idx] * 3;
            
            // 5. Additional arithmetic to create more RTL instructions
            for (int j = 0; j < 3; ++j) {
                mod[idx] += j;
            }
            
        } else {
            // ELSE BLOCK: Different operations
            mod[idx] = mod[idx] * 2;
            data[idx] = data[idx] - 1;
        }
        
        // Prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Additional loop to create more basic blocks
        int temp = 0;
        for (int k = 0; k < 2; ++k) {
            temp += data[(idx + k) % 1024];
        }
        accumulator += temp;
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
