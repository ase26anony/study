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
        // Test expression uses memory accesses with volatile
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test_expr
            
            // 1. Direct modification of data[idx] (part of test_expr)
            data[idx] += 1;
            
            // 2. Function call with address of mod[idx] (other part of test_expr)
            modify_value(&mod[idx], 2);
            
            // 3. Complex assignment that could alias test_expr
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = (*alias_ptr * 3) / 2;
            
            // 4. Another potential modification through pointer arithmetic
            mod[idx] = data[idx] / 2;
            
            // 5. Additional arithmetic to create more RTL instructions
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
            }
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] -= data[idx] % 5;
            data[idx] = mod[idx] * 2;
        }
        
        // Prevent dead code elimination
        global_accumulator += data[idx] + mod[idx];
        
        // Additional loop to create more basic blocks
        int temp = 0;
        for (int k = 0; k < 2; ++k) {
            temp += data[(idx + k) % SIZE];
        }
        global_accumulator ^= temp;
    }
    
    printf("Result: %d\n", global_accumulator);
    return 0;
}
