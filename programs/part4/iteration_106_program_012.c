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

// Complex condition helper
__attribute__((noinline, noclone))
int compute_threshold(int a, int b) {
    return (a ^ b) & 0xFF;
}

int main(void) {
    // Arrays with different storage classes
    static int data[1024];
    volatile int mod[1024];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < 1024; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    // Main computational loop - high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % 1024;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and arithmetic
        if (data[idx] > mod[idx] + compute_threshold(outer, idx)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] -= 2;
            
            // 2. Function call that takes address of mod[idx] - volatile part of test
            modify_value(&mod[idx], 1);
            
            // 3. Pointer arithmetic that could alias
            int* ptr = get_pointer(data, idx);
            *ptr = (*ptr * 3) / 2;
            
            // 4. Additional computation with mixed types
            float temp = (float)mod[idx];
            mod[idx] = (int)(temp * 1.1f);
            
            // 5. Memory store that might alias
            int alias_idx = idx ^ 1;
            if (alias_idx < 1024) {
                data[alias_idx] = mod[idx] / 2;
            }
            
            // 6. Complex assignment chain
            int old_val = mod[idx];
            mod[idx] = data[idx] + old_val % 10;
            data[idx] = old_val - 5;
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = data[idx] * 2;
            data[idx] = mod[idx] / 3;
            
            // Additional else-only operation
            if (idx % 3 == 0) {
                modify_value(&mod[idx], -1);
            }
        }
        
        // Prevent dead code elimination
        accumulator += data[idx] - mod[idx];
        
        // Additional loop computation to encourage if-conversion
        for (int inner = 0; inner < 3; ++inner) {
            int temp_idx = (idx + inner) % 1024;
            if (data[temp_idx] % 2 == 0) {
                accumulator += inner;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
