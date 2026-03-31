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

// Function to create complex then-block with multiple instructions
__attribute__((noinline, noclone))
void process_then_block(volatile int* mod, int* data, int idx, int* counter) {
    // Multiple operations that could potentially modify test expression components
    int temp = data[idx];
    temp = temp * 2 + 1;
    
    // Call to function with address-taking - complicates aliasing analysis
    modify_value(&mod[idx], temp);
    
    // Arithmetic that might be seen as modifying condition components
    data[idx] = (mod[idx] + data[idx]) / 2;
    
    // More operations to extend the basic block
    *counter += temp % 7;
    
    // Memory store that could alias with test expression
    mod[idx + 1] = data[idx] - 3;
}

int main() {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 100;
    }
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; outer++) {
        // Calculate index with wrap-around
        int idx = outer % (SIZE - 2);
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses memory accesses with volatile
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements creating several RTL instructions
            // This is what the uncovered code scans
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] = data[idx] - mod[idx];
            
            // 2. Function call with address of volatile mod[idx]
            modify_value(&mod[idx], data[idx] % 10);
            
            // 3. Complex computation that might alias
            int threshold = compute_threshold(data[idx], idx);
            
            // 4. Assignment that could modify condition components
            // This is key: mod[idx] appears in test expression
            mod[idx] = data[idx] / 2 + threshold;
            
            // 5. More operations to extend basic block
            process_then_block(mod, data, idx, &accumulator);
            
            // 6. Final operation in then block
            data[idx + 1] = mod[idx] * 2;
            
        } else {
            // ELSE BLOCK: Different operations to keep it live
            mod[idx] = data[idx] + 5;
            data[idx] = mod[idx] * 3 % 100;
            accumulator -= 1;
        }
        
        // Additional operations to prevent dead code elimination
        int result = data[idx] + mod[idx];
        accumulator += result % 19;
        
        // Mix in some floating point to complicate RTL
        float ftemp = (float)data[idx] / (mod[idx] + 1);
        accumulator += (int)(ftemp * 10);
    }
    
    printf("Final accumulator: %d\n", accumulator);
    return 0;
}
