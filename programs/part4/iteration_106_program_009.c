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

// Global volatile to prevent optimizations
volatile int global_counter = 0;

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    volatile int accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access to volatile and non-volatile arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] = data[idx] - 1;
            
            // 2. Function call with pointer to mod[idx] - could alias
            modify_value(&mod[idx], 2);
            
            // 3. Complex assignment that might be seen as modifying test expression
            //    through pointer arithmetic or aliasing
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = (*alias_ptr) * 2;
            
            // 4. Another operation on mod[idx]
            mod[idx] = data[idx] / 3 + mod[idx] % 7;
            
            // 5. Mixed type operation to complicate RTL
            float temp = (float)data[idx];
            mod[idx] += (int)(temp * 0.5f);
            
            // 6. Sequence of arithmetic operations
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
                mod[idx] -= j;
            }
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            data[idx] = data[idx] + mod[idx];
            mod[idx] = mod[idx] - 5;
            
            // Call to prevent optimization
            modify_value(&mod[idx], -1);
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] - mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 100 == 0) {
            int new_idx = (idx + 1) % SIZE;
            data[new_idx] = accumulator % 100;
        }
        
        // Volatile operation to prevent reordering
        global_counter = outer;
    }
    
    // Use results to prevent optimization
    printf("Result: accumulator = %d, global_counter = %d\n", 
           accumulator, global_counter);
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
