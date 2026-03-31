#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int val) {
    *ptr = val;
}

// Another opaque function that might alias
__attribute__((noinline, noclone))
int* get_pointer(int* base, int idx) {
    return &base[idx];
}

// Global volatile to prevent dead code elimination
volatile int global_accumulator = 0;

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory accesses to both arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test_expr
            
            // 1. Direct modification of data[idx] - part of test_expr
            data[idx] += 1;
            
            // 2. Function call with address of mod[idx] - volatile part of test_expr
            modify_value(&mod[idx], data[idx] / 2);
            
            // 3. Complex assignment that might alias
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = (*alias_ptr) * 3 / 2;
            
            // 4. Additional arithmetic to create more RTL instructions
            for (int j = 0; j < 3; ++j) {
                mod[idx] = mod[idx] - j;
            }
            
            // 5. Memory store that could be seen as modifying test_expr
            volatile int* volatile_mod_ptr = &mod[idx];
            *volatile_mod_ptr = data[idx] + 10;
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = data[idx] * 2;
            data[idx] = mod[idx] / 3;
        }
        
        // Prevent dead code elimination
        global_accumulator += data[idx] + mod[idx];
        
        // Additional loop to create more complex CFG
        int temp = 0;
        for (int k = 0; k < 5; ++k) {
            temp += data[(idx + k) % SIZE];
        }
        global_accumulator ^= temp;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
