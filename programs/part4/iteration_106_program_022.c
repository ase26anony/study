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

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    
    // Initialize arrays with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 2) % 100;
    }
    
    volatile int accumulator = 0;
    volatile int control_var = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index - complex enough to prevent optimization
        int idx = (outer * 7 + control_var) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and arithmetic
        if (data[idx] > mod[idx] + compute_threshold(control_var, idx)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] += 2;
            
            // 2. Function call with pointer to mod[idx] (aliasing possibility)
            modify_value(&mod[idx], 1);
            
            // 3. Arithmetic that could be seen as modifying test expression
            int temp = data[idx] * 3;
            mod[idx] = temp / 4;  // This modifies mod[idx] from test expression
            
            // 4. More operations to create multiple basic blocks
            for (int j = 0; j < 3; ++j) {
                data[idx] -= j;
            }
            
            // 5. Another function call
            modify_value(&mod[idx], compute_threshold(idx, outer));
            
            // 6. Final assignment
            data[idx] = mod[idx] + 5;
            
        } else {
            // ELSE BLOCK: Different operations
            mod[idx] = data[idx] - 10;
            data[idx] = mod[idx] * 2;
        }
        
        // Prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Modify control variable to affect loop behavior
        control_var = (control_var + accumulator) % 100;
        
        // Additional computation to keep loop non-trivial
        for (int k = 0; k < 4; ++k) {
            accumulator += k * data[idx % SIZE];
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
