#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int val) {
    *ptr = val;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_value(int a, int b) {
    return a + b * 2;
}

// Global volatile to prevent optimizations
volatile int global_counter = 0;

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    
    // Initialize arrays with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 150;
    }
    
    volatile int accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 7 + 13) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses memory accesses with mixed types
        if (data[idx] > (int)mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] = data[idx] - 1;
            
            // 2. Function call with address of mod[idx] (volatile)
            modify_value(&mod[idx], data[idx] * 2);
            
            // 3. Complex assignment that could alias
            int temp = compute_value(data[idx], mod[idx]);
            
            // 4. Another potential modification
            mod[idx] = temp / 3 + 1;
            
            // 5. Additional arithmetic
            data[idx] = data[idx] + mod[idx] % 10;
            
            // 6. Global side effect
            global_counter++;
            
            // 7. Pointer arithmetic that could confuse analysis
            int* data_ptr = &data[idx];
            *data_ptr = *data_ptr + (mod[idx] > 50 ? 1 : -1);
            
        } else {
            // ELSE BLOCK: Different operations
            mod[idx] = mod[idx] + 5;
            data[idx] = data[idx] * 2;
            
            // Call to prevent optimization
            int temp = compute_value(mod[idx], data[idx]);
            mod[idx] = temp % 100;
        }
        
        // Additional operations to prevent dead code elimination
        accumulator += data[idx] - mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 100 == 0) {
            // Nested condition to create more basic blocks
            int alt_idx = (idx + 1) % SIZE;
            if (data[alt_idx] < mod[alt_idx] * 2) {
                data[alt_idx] += 2;
                modify_value(&mod[alt_idx], data[alt_idx] / 2);
            }
        }
        
        // Mix in floating point to complicate RTL
        float fval = (float)data[idx] / (mod[idx] + 1);
        if (fval > 0.5f) {
            accumulator += (int)(fval * 10);
        }
    }
    
    printf("Result: %d (global: %d)\n", accumulator, global_counter);
    return 0;
}
