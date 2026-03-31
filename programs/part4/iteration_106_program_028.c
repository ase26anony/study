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
int compute_threshold(int a, int b) {
    return (a + b) / 3;
}

int main(void) {
    // Initialize arrays with pattern
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    volatile int accumulator = 0;
    volatile int control_var = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < ITERATIONS; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 31) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Complex test expression involving memory access and computation
        if (data[idx] > mod[idx] + control_var) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Modify data[idx] - part of test expression
            data[idx] = data[idx] - 1;
            
            // 2. Call opaque function with address of mod[idx] 
            // (could alias with test expression)
            modify_value(&mod[idx], data[idx] * 2);
            
            // 3. Complex computation that might be seen as aliasing
            int temp = compute_threshold(data[idx], mod[idx]);
            
            // 4. Assignment that modifies mod[idx] - part of test expression
            mod[idx] = data[idx] / 2 + temp;
            
            // 5. Additional operations to create more instructions
            control_var = (control_var + 1) % 10;
            data[(idx + 1) % SIZE] = temp;
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = mod[idx] + 5;
            data[idx] = data[idx] * 2;
            control_var = (control_var - 1) % 10;
        }
        
        // Additional operations to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Complex loop with multiple basic blocks
        for (int inner = 0; inner < 3; ++inner) {
            int inner_idx = (idx + inner) % SIZE;
            if (data[inner_idx] % 2 == 0) {
                mod[inner_idx] = mod[inner_idx] + inner;
            } else {
                data[inner_idx] = data[inner_idx] - inner;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
