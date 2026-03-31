#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int delta) {
    *ptr += delta;
}

// Another opaque function that could alias
__attribute__((noinline, noclone))
int* get_data_ptr(int* base, int idx) {
    return &base[idx];
}

// Complex condition helper
__attribute__((noinline, noclone))
int compute_threshold(int a, int b) {
    return (a * 3 + b) / 2;
}

int main(void) {
    // Arrays with different storage classes
    static int data[1024];
    volatile int mod[1024];
    int temp_buffer[1024];
    
    // Initialize with pattern
    for (int i = 0; i < 1024; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
        temp_buffer[i] = 0;
    }
    
    volatile int accumulator = 0;
    volatile int control = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + control) % 1024;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Complex test expression involving memory accesses and type mixing
        if (data[idx] > (int)mod[idx] + compute_threshold(control, idx)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of memory used in condition
            data[idx] -= 5;
            
            // 2. Function call that takes address of volatile mod[idx]
            modify_value(&mod[idx], 2);
            
            // 3. Assignment that creates potential aliasing
            int* alias_ptr = get_data_ptr(data, idx);
            *alias_ptr = (*alias_ptr * 2) % 100;
            
            // 4. More arithmetic that could affect the test expression
            temp_buffer[idx] = data[idx] + mod[idx];
            
            // 5. Complex operation with side effects
            for (int j = 0; j < 3; ++j) {
                mod[idx] = mod[idx] ^ (1 << j);
            }
            
            // 6. Final assignment that looks like it could modify test expression
            mod[idx] = data[idx] / 2 + temp_buffer[idx];
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            data[idx] += 3;
            mod[idx] -= 1;
            temp_buffer[idx] = data[idx] - mod[idx];
        }
        
        // Additional operations to prevent dead code elimination
        accumulator += data[idx] ^ mod[idx];
        control = (control + 1) % 100;
        
        // Another conditional to create more basic blocks
        if (outer % 1000 == 0) {
            modify_value(&mod[idx], accumulator % 10);
        }
    }
    
    // Use results
    printf("Result: %d\n", accumulator);
    
    // Verify some values to prevent complete optimization
    int sum = 0;
    for (int i = 0; i < 1024; ++i) {
        sum += data[i] + mod[i];
    }
    printf("Checksum: %d\n", sum);
    
    return 0;
}
