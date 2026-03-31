#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int delta) {
    *ptr += delta;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int base, int idx) {
    return (base + idx) % 50;
}

// Global volatile to prevent optimization
volatile int global_counter = 0;

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 30);
    }
    
    volatile int accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory accesses and arithmetic
        if (data[idx] > mod[idx] + compute_threshold(global_counter, idx)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] -= 2;
            
            // 2. Function call that takes address of mod[idx] (volatile)
            modify_value(&mod[idx], 1);
            
            // 3. Complex assignment that could alias
            int temp = data[idx] * 3;
            mod[idx] = temp / 4 + (idx % 3);
            
            // 4. Another computation
            data[idx] = (data[idx] + mod[idx]) / 2;
            
            // 5. Additional statements to create more RTL instructions
            global_counter += idx % 7;
            accumulator += data[idx] - mod[idx];
            
        } else {
            // ELSE BLOCK: Different operations
            data[idx] += 3;
            mod[idx] -= 1;
            accumulator -= data[idx] + mod[idx];
        }
        
        // Additional operations to prevent dead code elimination
        if (idx % 128 == 0) {
            // Nested condition to create more complex CFG
            int temp_idx = (idx + 1) % SIZE;
            if (data[temp_idx] < mod[temp_idx]) {
                data[temp_idx] = mod[temp_idx] * 2;
                modify_value(&mod[temp_idx], -1);
            }
        }
        
        // Mix data types in computations
        float float_val = data[idx] * 0.5f;
        double double_val = mod[idx] * 0.25;
        if (float_val > double_val) {
            accumulator += 1;
        }
    }
    
    // Use results
    int final_sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        final_sum += data[i] + mod[i];
    }
    
    printf("Result: accumulator=%d, final_sum=%d, global_counter=%d\n", 
           accumulator, final_sum, global_counter);
    
    return 0;
}
