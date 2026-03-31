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

// Complex test expression helper
__attribute__((noinline, noclone))
int load_and_transform(volatile int* arr, int idx) {
    return arr[idx] * 2 + 1;
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
    
    // Main computational loop - high trip count encourages if-conversion
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % 1024;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and volatile
        if (data[idx] > load_and_transform(mod, idx)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] += 2;
            
            // 2. Function call with address of mod[idx] - volatile, part of test
            modify_value(&mod[idx], -1);
            
            // 3. Assignment that could alias - using pointer from function
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = (*alias_ptr % 100) + 1;
            
            // 4. Arithmetic with potential side effects
            volatile int temp = mod[idx];
            mod[idx] = data[idx] / (temp > 0 ? temp : 1);
            
            // 5. Another operation on test expression component
            data[idx] = data[idx] ^ 0x01;
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = mod[idx] * 3 / 2;
            data[idx] -= 1;
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] - mod[idx];
        
        // Complex loop with multiple exit points to create interesting CFG
        if (outer % 1000 == 0) {
            // Force compiler to consider both paths
            volatile int r = rand() % 100;
            if (r < 30) {
                data[idx] = r;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
