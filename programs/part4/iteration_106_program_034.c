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
    return (base ^ idx) & 0x7F;
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
        mod[i] = 50 + (i % 50);
    }
    
    volatile int accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index - complex enough to prevent hoisting
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and computation
        if (data[idx] > (mod[idx] + compute_threshold(outer, idx))) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Modify data[idx] - part of test expression
            data[idx] = data[idx] - (mod[idx] % 10);
            
            // 2. Call function that takes address of mod[idx] 
            // (could alias with test expression)
            modify_value(&mod[idx], data[idx] & 3);
            
            // 3. Complex assignment that might be seen as aliasing
            int temp = data[idx];
            mod[idx] = temp / 2 + (temp % 7);
            
            // 4. More arithmetic
            data[idx] = (data[idx] * 3 + 1) % 256;
            
            // 5. Function call with potential side effects
            global_counter += (data[idx] > 100) ? 1 : 0;
            
            // 6. Final assignment that uses both variables
            mod[idx] = data[idx] + (mod[idx] % 20);
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = mod[idx] - (data[idx] % 5);
            data[idx] = (data[idx] + mod[idx]) & 0xFF;
            
            // Additional else-only operation
            if (mod[idx] < 0) {
                mod[idx] = -mod[idx];
            }
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] ^ mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 100 == 0) {
            // This creates additional control flow
            int alt_idx = (idx + 1) % SIZE;
            if (data[alt_idx] > mod[alt_idx]) {
                data[alt_idx] >>= 1;
            }
        }
    }
    
    printf("Result: %d (global: %d)\n", accumulator, global_counter);
    
    // Verify some values to prevent complete optimization
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
