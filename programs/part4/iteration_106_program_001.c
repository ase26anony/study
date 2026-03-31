#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int delta) {
    *ptr += delta;
}

// Another opaque function that might alias
__attribute__((noinline, noclone))
int* get_pointer(int* base, int idx) {
    return &base[idx];
}

int main(void) {
    // Arrays with different storage classes
    int data[1024];
    volatile int mod[1024];  // volatile to prevent optimization
    int* alias_ptr = NULL;
    
    // Initialize with pattern
    for (int i = 0; i < 1024; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 100;
    }
    
    volatile int accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % 1024;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access to volatile and non-volatile arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] (part of test expression)
            data[idx] += 1;
            
            // 2. Function call that takes address of mod[idx] (other part of test expression)
            modify_value(&mod[idx], 2);
            
            // 3. Get pointer that might alias with test expression
            alias_ptr = get_pointer(data, idx);
            
            // 4. Assignment through potentially aliasing pointer
            if (alias_ptr) {
                *alias_ptr = *alias_ptr / 2;
            }
            
            // 5. Another operation on mod[idx]
            mod[idx] = data[idx] + mod[idx] / 3;
            
            // 6. Mixed type operation to complicate RTL
            float temp = (float)data[idx];
            mod[idx] += (int)(temp * 0.5f);
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] -= data[idx] % 7;
            data[idx] = mod[idx] * 2;
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] - mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 100 == 0) {
            // Occasionally modify the arrays in a way that might affect future iterations
            int j = (outer / 100) % 1024;
            data[j] = mod[j] + outer;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
