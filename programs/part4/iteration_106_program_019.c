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

// Global volatile to prevent optimizations
volatile int global_counter = 0;

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    
    // Initialize arrays with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 100;
    }
    
    volatile int accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 31) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and computation
        if (data[idx] > mod[idx] + compute_threshold(outer, idx)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] = data[idx] - 1;
            
            // 2. Function call with address of mod[idx] (aliasing concern)
            modify_value(&mod[idx], 2);
            
            // 3. Complex assignment that could be seen as aliasing
            int temp = data[idx];
            mod[idx] = temp / 2 + (idx % 3);
            
            // 4. Additional arithmetic to create more instructions
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
            }
            
            // 5. Another function call
            int threshold = compute_threshold(data[idx], idx);
            if (threshold > 25) {
                mod[idx] = threshold;
            }
            
            // 6. Final assignment
            data[idx] = mod[idx] * 2;
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = data[idx] + 5;
            data[idx] = mod[idx] / 3;
        }
        
        // Prevent dead-code elimination
        accumulator += data[idx] - mod[idx];
        
        // Additional loop to create more basic blocks
        for (int k = 0; k < 2; ++k) {
            if (k == 0) {
                data[idx] += accumulator % 7;
            } else {
                mod[idx] -= accumulator % 5;
            }
        }
        
        // Mix data types to complicate RTL
        float f_temp = data[idx];
        if (f_temp > 50.0f) {
            mod[idx] = (int)(f_temp * 0.8f);
        }
        
        // Global side effect
        global_counter += (data[idx] > mod[idx]) ? 1 : -1;
    }
    
    printf("Result: accumulator = %d, global_counter = %d\n", 
           accumulator, global_counter);
    
    // Verify some values to prevent complete optimization
    int verify_sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        verify_sum += data[i];
    }
    printf("Data sum: %d\n", verify_sum);
    
    return 0;
}
