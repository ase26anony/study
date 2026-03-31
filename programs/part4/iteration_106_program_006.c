#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int val) {
    *ptr = val;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_value(int a, int b) {
    return a + b * 2;
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
        mod[i] = (i % 50) + 25;
    }
    
    volatile int accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 31) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access to both arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Modify data[idx] - could affect test expression
            data[idx] = data[idx] - 1;
            
            // 2. Call opaque function with address of mod[idx]
            // This creates complex aliasing that modified_in_p must analyze
            modify_value(&mod[idx], compute_value(data[idx], idx));
            
            // 3. Additional arithmetic that might alias
            int temp = data[idx] * 2;
            
            // 4. Another assignment that could affect the test expression
            // Using division to create different RTL pattern
            mod[idx] = data[idx] / 2 + temp % 3;
            
            // 5. More operations to extend the basic block
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
            }
            
            // 6. Final modification that definitely affects test expression
            data[idx] = data[idx] % 100;
            
        } else {
            // ELSE BLOCK: Different operations to keep both paths live
            mod[idx] = mod[idx] + 5;
            data[idx] = data[idx] * 2;
            
            // Additional else operations
            if (mod[idx] > 100) {
                mod[idx] = 100;
            }
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] - mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 1000 == 0) {
            global_counter = accumulator;
        }
    }
    
    // Use results to prevent optimization
    printf("Result: accumulator = %d, global_counter = %d\n", 
           accumulator, global_counter);
    
    // Verify array state
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += data[i];
    }
    printf("Array sum: %d\n", sum);
    
    return 0;
}
