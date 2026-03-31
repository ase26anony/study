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
    return arr[idx] * 2 - 1;
}

int main(void) {
    // Arrays with different storage classes
    int data[1024];
    volatile int mod[1024];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < 1024; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 150;
    }
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 7 + 13) % 1024;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses mixed memory accesses
        if (data[idx] > load_and_transform(mod, idx)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] += 2;
            
            // 2. Function call that takes address of mod[idx] 
            // (volatile, part of test expression via load_and_transform)
            modify_value(&mod[idx], 1);
            
            // 3. Assignment that creates potential aliasing
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = (*alias_ptr % 50) + 10;
            
            // 4. Arithmetic that could be seen as modifying memory
            // through different access pattern
            int temp = mod[idx];
            mod[idx] = data[idx] / (temp > 0 ? temp : 1);
            
            // 5. Additional computation to extend basic block
            for (int j = 0; j < 3; ++j) {
                data[idx] ^= (1 << j);
            }
            
            // 6. Final assignment that completes the then block
            mod[idx] = data[idx] * 2 - mod[idx];
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] -= data[idx] % 20;
            data[idx] = (data[idx] + mod[idx]) / 2;
        }
        
        // Prevent dead code elimination
        accumulator += data[idx] ^ mod[idx];
        
        // Additional loop to create more RTL instructions
        for (int k = 0; k < 2; ++k) {
            accumulator ^= (data[(idx + k) % 1024] << k);
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
