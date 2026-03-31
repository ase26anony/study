#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int delta) {
    *ptr += delta;
}

// Another opaque function for aliasing complexity
__attribute__((noinline, noclone))
int* get_pointer(int* base, int idx) {
    return &base[idx];
}

// Global volatile to prevent dead code elimination
volatile int global_accumulator = 0;

int main() {
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
        // Test expression uses memory accesses with mixed types
        if (data[idx] > (int)mod[idx]) {
            // THEN BLOCK - Multiple statements that could modify test_expr
            
            // 1. Direct modification of data[idx] (part of test_expr)
            data[idx] = data[idx] - 1;
            
            // 2. Function call with pointer to mod[idx] (other part of test_expr)
            modify_value(&mod[idx], 2);
            
            // 3. Complex assignment that creates aliasing concerns
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = (*alias_ptr) * 2;
            
            // 4. Another potential modification through different access
            mod[idx] = data[idx] / 3;
            
            // 5. Additional arithmetic to create more RTL instructions
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
            }
            
            // 6. Memory barrier-like operation
            asm volatile("" : : : "memory");
            
        } else {
            // ELSE BLOCK - Different operations to keep path alive
            mod[idx] = mod[idx] * 2;
            data[idx] = data[idx] + 5;
        }
        
        // Prevent dead code elimination
        global_accumulator += data[idx] + mod[idx];
        
        // Additional loop to create more basic blocks
        int temp = 0;
        for (int k = 0; k < 2; ++k) {
            temp += data[(idx + k) % SIZE];
        }
        global_accumulator += temp;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
