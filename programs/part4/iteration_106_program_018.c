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
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 100;
    }
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 7 + 13) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses memory accesses with volatile
        if (data[idx] > mod[idx] && 
            load_and_transform(mod, idx) < 200) {  // Complex test
            
            // THEN BLOCK: Multiple statements that could modify test expression
            // This should generate multiple RTL instructions
            int temp = data[idx];
            
            // 1. Arithmetic that could affect test expression
            data[idx] = temp + 1;
            
            // 2. Function call with potential aliasing
            modify_value(&mod[idx], 1);
            
            // 3. Assignment that uses test expression component
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = (*alias_ptr) * 2 % 100;
            
            // 4. Another operation on test expression variable
            if (mod[idx] > 50) {
                mod[idx] = mod[idx] / 2;
            }
            
            // 5. Complex sequence increasing instruction count
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
            }
            
            // Final operation that definitely modifies test expression component
            mod[idx] = data[idx] % 100;
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = data[idx] + 5;
            data[idx] = (data[idx] * 3) % 100;
        }
        
        // Prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Additional loop to create more basic blocks
        int sum = 0;
        for (int k = 0; k < 4; ++k) {
            sum += data[(idx + k) % SIZE];
        }
        accumulator ^= sum;
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
