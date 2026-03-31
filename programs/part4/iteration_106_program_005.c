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
    return (base + idx) % 50;
}

// Global volatile to prevent optimization
volatile int global_counter = 0;

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 100;
    }
    
    volatile int accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 31) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and volatile
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] = data[idx] - 1;
            
            // 2. Function call that takes address of mod[idx] (volatile part of test)
            modify_value(&mod[idx], 2);
            
            // 3. Complex assignment that could alias
            int temp = compute_threshold(data[idx], idx);
            
            // 4. Assignment to mod[idx] using data[idx] - modifies volatile part
            mod[idx] = data[idx] / 2 + temp;
            
            // 5. Additional arithmetic
            data[idx] = data[idx] * 3 % 100;
            
            // 6. Another function call
            modify_value(&mod[idx], -1);
            
            // 7. Final computation
            accumulator += data[idx] + mod[idx];
            
        } else {
            // ELSE BLOCK: Different operations
            data[idx] = data[idx] + 5;
            mod[idx] = mod[idx] - 3;
            accumulator -= data[idx] - mod[idx];
        }
        
        // Additional operations to prevent dead code elimination
        if (outer % 1000 == 0) {
            global_counter += accumulator;
        }
        
        // Mix in different data types
        float f_idx = idx;
        if (f_idx > 500.0f) {
            data[idx] = (int)(f_idx * 0.5f);
        }
    }
    
    printf("Final accumulator: %d\n", accumulator);
    printf("Global counter: %d\n", global_counter);
    
    // Verify results aren't optimized away
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
