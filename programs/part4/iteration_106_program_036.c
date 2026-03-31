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
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    volatile long long accumulator = 0;
    
    // Outer loop with high trip count to encourage if-conversion
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with some variation
        int idx = (outer * 17) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory accesses to both arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test_expr
            
            // 1. Direct modification of data[idx] - part of test_expr
            data[idx] = data[idx] - 10;
            
            // 2. Function call that takes address of mod[idx] - volatile part of test_expr
            modify_value(&mod[idx], 5);
            
            // 3. Complex assignment that could be seen as aliasing
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = (*alias_ptr) * 2;
            
            // 4. Another operation on mod[idx]
            mod[idx] = data[idx] / 2;
            
            // 5. Additional arithmetic to create more RTL instructions
            int temp = mod[idx] + data[idx];
            mod[idx] = temp % 100;
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            data[idx] = data[idx] + 5;
            mod[idx] = mod[idx] - 3;
            
            // Additional else-block operations
            int temp = data[idx] * 2;
            if (temp > 100) {
                mod[idx] = mod[idx] + temp;
            }
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Mix data types to complicate RTL representation
        float f_val = data[idx] * 0.5f;
        if (f_val > 25.0f) {
            accumulator += (long long)f_val;
        }
    }
    
    // Nested loop with different test pattern
    for (int i = 0; i < SIZE; ++i) {
        // Another if-then-else with pointer arithmetic
        int* data_ptr = &data[i];
        volatile int* mod_ptr = &mod[i];
        
        if (*data_ptr > *mod_ptr) {
            // Multiple modifications in then block
            *data_ptr = *data_ptr - *mod_ptr;
            modify_value(mod_ptr, *data_ptr);
            
            // Pointer chase that could alias
            int* another_ptr = get_pointer(data, i);
            *another_ptr = (*another_ptr) ^ 0xFF;
            
            *mod_ptr = *data_ptr << 1;
        } else {
            *mod_ptr = *data_ptr + 10;
        }
        
        accumulator += *data_ptr;
    }
    
    printf("Result: %lld\n", accumulator);
    return 0;
}
