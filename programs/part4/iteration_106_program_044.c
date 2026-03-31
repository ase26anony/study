#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int value) {
    *ptr = value;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int base, int offset) {
    return (base * 3 + offset) % 100;
}

int main(void) {
    // Arrays with different storage classes
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = (i * 2) % 100;
    }
    
    // Main loop - this is where if-conversion will be attempted
    for (int outer = 0; outer < ITERATIONS; outer++) {
        // Calculate index with complex expression
        int idx = (outer * 7 + 13) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access to volatile and non-volatile arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] = data[idx] + 1;
            
            // 2. Function call that takes address of mod[idx] - could alias
            modify_value(&mod[idx], compute_threshold(data[idx], idx));
            
            // 3. Complex assignment that might be seen as aliasing
            int temp = data[idx];
            mod[idx] = temp / 2 + (idx % 3);
            
            // 4. More arithmetic
            data[idx] = data[idx] - (mod[idx] % 5);
            
            // 5. Another function call
            int new_val = compute_threshold(mod[idx], data[idx]);
            if (new_val > 50) {
                mod[idx] = new_val;
            }
            
        } else {
            // ELSE BLOCK: Different operations to ensure both paths are live
            mod[idx] = mod[idx] * 2;
            if (mod[idx] > 100) {
                mod[idx] = mod[idx] % 100;
            }
            data[idx] = data[idx] - 1;
        }
        
        // Prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Additional loop to create more basic blocks
        for (int inner = 0; inner < 3; inner++) {
            if (data[(idx + inner) % SIZE] % 2 == 0) {
                mod[(idx + inner) % SIZE] += inner;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
