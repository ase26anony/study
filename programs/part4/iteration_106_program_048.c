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
    return (base * idx) % 100;
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
        mod[i] = 50 + (i % 50);
    }
    
    // Volatile accumulator to prevent dead code elimination
    volatile long long accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and volatile
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] = data[idx] - 10;
            
            // 2. Function call that takes address of mod[idx] (volatile part of condition)
            modify_value(&mod[idx], 5);
            
            // 3. Complex assignment that could alias with condition
            int temp = data[idx] + mod[idx];
            mod[idx] = temp / 2;  // This modifies volatile part of condition
            
            // 4. More arithmetic operations
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
            }
            
            // 5. Another function call with computed value
            int threshold = compute_threshold(data[idx], idx);
            if (threshold > 75) {
                mod[idx] = threshold;
            }
            
            // 6. Final assignment
            data[idx] = mod[idx] * 2;
            
        } else {
            // ELSE BLOCK: Different operations to ensure both paths are live
            mod[idx] = data[idx] + 1;
            data[idx] = mod[idx] * 3;
            
            // Additional computation to match complexity
            int temp = compute_threshold(mod[idx], idx);
            if (temp < 25) {
                data[idx] = temp;
            }
        }
        
        // Additional operations to prevent optimization
        accumulator += data[idx] + mod[idx];
        
        // Periodic modification to break patterns
        if (outer % 1000 == 0) {
            global_counter++;
            mod[idx] = global_counter;
        }
    }
    
    printf("Result: %lld\n", accumulator);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
