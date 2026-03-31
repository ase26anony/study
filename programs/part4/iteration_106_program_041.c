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
    return (base + idx * 3) % 256;
}

// Global volatile to force memory operations
volatile int global_counter = 0;

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 70);
    }
    
    volatile int accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and arithmetic
        if (data[idx] > mod[idx] + compute_threshold(outer, idx)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] = data[idx] - (mod[idx] % 10);
            
            // 2. Function call with address of mod[idx] - could alias
            modify_value(&mod[idx], data[idx] % 5);
            
            // 3. Complex assignment involving both sides of test
            int temp = data[idx] * 2;
            mod[idx] = temp / 3 + (idx % 7);
            
            // 4. Another operation on data[idx]
            data[idx] ^= (mod[idx] & 0xFF);
            
            // 5. Function call that might affect memory
            global_counter++;
            
            // 6. Final assignment that completes the sequence
            mod[idx] = data[idx] + (idx % 20);
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = data[idx] * 2;
            data[idx] = mod[idx] - 100;
            global_counter--;
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 1000 == 0) {
            // This creates additional control flow
            for (int j = 0; j < 10; ++j) {
                data[(idx + j) % SIZE] += accumulator % 100;
            }
        }
    }
    
    printf("Result: %d (global: %d)\n", accumulator, global_counter);
    return 0;
}
