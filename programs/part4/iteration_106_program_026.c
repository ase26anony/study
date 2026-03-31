#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int delta) {
    *ptr += delta;
}

// Another opaque function that could alias
__attribute__((noinline, noclone))
int* get_pointer(int* base, int idx) {
    return &base[idx];
}

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    
    // Initialize arrays with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 100;
    }
    
    volatile int accumulator = 0;
    int outer_iterations = 1000000;
    
    // Outer loop to encourage if-conversion analysis
    for (int iter = 0; iter < outer_iterations; ++iter) {
        // Calculate index - complex enough to prevent hoisting
        int idx = (iter * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory accesses to both arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test_expr
            // 1. Direct modification of data[idx] used in condition
            data[idx] += 1;
            
            // 2. Function call that takes address of mod[idx]
            // This creates complex RTL patterns
            modify_value(&mod[idx], 2);
            
            // 3. Assignment that could be seen as aliasing
            // Using pointer arithmetic to potentially alias
            int* ptr = get_pointer(data, idx);
            *ptr = (*ptr) / 2 + 1;
            
            // 4. Additional arithmetic to create more instructions
            int temp = mod[idx];
            for (int j = 0; j < 3; ++j) {
                temp += j;
            }
            mod[idx] = temp;
            
            // 5. Memory store that might alias with test expression
            data[(idx + 1) % SIZE] = mod[idx] - 5;
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = data[idx] * 2;
            data[idx] -= 1;
            
            // Additional computation to match complexity
            int temp = mod[idx];
            for (int j = 0; j < 2; ++j) {
                temp -= j;
            }
            mod[idx] = temp;
        }
        
        // Prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Additional loop to create more basic blocks
        for (int k = 0; k < 2; ++k) {
            accumulator += k;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
