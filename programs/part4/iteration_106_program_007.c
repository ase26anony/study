#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 1000000

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr) {
    *ptr += 1;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int x, int y) {
    return (x + y) % 100;
}

int main() {
    int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 30);
    }
    
    // Main loop - designed to trigger if-conversion analysis
    for (int outer = 0; outer < ITERATIONS; outer++) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory accesses to both arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] = data[idx] - 1;
            
            // 2. Function call with address of mod[idx] - could alias
            modify_value(&mod[idx]);
            
            // 3. Assignment that uses both sides of the original condition
            int temp = compute_threshold(data[idx], mod[idx]);
            
            // 4. Another potential modification
            if (temp > 75) {
                mod[idx] = data[idx] / 2;  // This modifies mod[idx]!
            }
            
            // 5. More arithmetic
            data[idx] += temp % 10;
            
            // 6. Complex sequence to create multiple basic blocks
            for (int j = 0; j < 3; j++) {
                mod[idx] += j;
            }
            
        } else {
            // ELSE BLOCK: Different operations to keep both paths live
            int new_val = mod[idx] + data[idx];
            data[idx] = new_val % 100;
            
            // Call to prevent optimization
            modify_value(&mod[idx]);
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 1000 == 0) {
            // Reset some values periodically
            int reset_idx = (outer / 1000) % SIZE;
            mod[reset_idx] = 50;
            data[reset_idx] = 75;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
