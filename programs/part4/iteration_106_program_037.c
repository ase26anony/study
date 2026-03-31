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
    return (base + idx * 3) % 100;
}

// Function to create complex then-block with potential modification
__attribute__((noinline, noclone))
void process_then_block(volatile int* mod, int* data, int idx, int* counter) {
    // Multiple operations to create several RTL instructions
    int temp = data[idx];
    temp = temp * 2 + 1;
    
    // Call to opaque function - could affect aliasing analysis
    modify_value(&mod[idx], temp % 10);
    
    // Arithmetic that might be seen as modifying test expression
    data[idx] = temp - (mod[idx] / 2);
    
    // More operations to extend basic block
    *counter += data[idx] & 0xF;
    mod[idx] = (*counter % 256) | 0x1000;
    
    // Function call that takes address of condition variable
    modify_value(&mod[idx], idx % 7);
}

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 100;
    }
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; outer++) {
        int idx = outer % SIZE;
        int local_counter = 0;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses memory accesses with mixed types
        if ((int)data[idx] > (unsigned int)mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            // This should create several RTL instructions before block boundary
            
            // 1. Direct modification of data[idx] used in condition
            int old_val = data[idx];
            data[idx] = old_val - (mod[idx] % 5);
            
            // 2. Call to function with complex operations
            process_then_block(mod, data, idx, &local_counter);
            
            // 3. More arithmetic that might alias
            mod[idx] = (data[idx] * 2 + mod[idx]) % 256;
            
            // 4. Additional operations to extend the block
            for (int j = 0; j < 3; j++) {
                data[idx] += j;
                local_counter++;
            }
            
            // 5. Final assignment that could affect condition
            mod[idx] = compute_threshold(data[idx], idx);
            
        } else {
            // ELSE BLOCK: Simpler path but still live
            mod[idx] = data[idx] + 1;
            data[idx] = mod[idx] * 2;
        }
        
        // Additional operations to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Complex loop-dependent computation
        if (outer % 1000 == 0) {
            // Force memory barrier-like effect
            asm volatile("" : : "r"(data), "r"(mod) : "memory");
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
