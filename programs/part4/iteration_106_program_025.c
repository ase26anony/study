#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    return (x * 3 + y) / 2;
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
    int threshold = 75;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < ITERATIONS; ++outer) {
        // Calculate index with wrap-around
        int idx = outer % SIZE;
        
        // Complex test expression involving memory access and computation
        // This creates a test_expr that if-conversion will analyze
        int test_val = data[idx];
        int mod_val = mod[idx];
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // The condition uses variables that could be modified in the then block
        if (test_val > mod_val) {
            // THEN BLOCK: Multiple statements that could modify test_expr components
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] = data[idx] + 1;
            
            // 2. Function call that takes address of mod[idx] - could alias
            modify_value(&mod[idx]);
            
            // 3. Assignment that uses data[idx] after modification
            int temp = data[idx] * 2;
            
            // 4. Another computation that writes to mod[idx]
            mod[idx] = compute_threshold(data[idx], mod[idx]);
            
            // 5. Additional arithmetic to create more RTL instructions
            for (int j = 0; j < 3; ++j) {
                temp += j;
            }
            
            // 6. Final assignment that could be seen as modifying
            // something related to the original test expression
            if (temp > 100) {
                mod[idx] = temp % 100;
            }
            
        } else {
            // ELSE BLOCK: Different operations to keep both paths live
            mod[idx] = mod[idx] - 1;
            data[idx] = data[idx] * 2;
            
            // Additional else-block operations
            int temp = mod[idx];
            for (int j = 0; j < 2; ++j) {
                temp -= j;
            }
            mod[idx] = temp;
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Mix data types to complicate RTL representation
        if (outer % 100 == 0) {
            float fval = (float)data[idx];
            double dval = (double)mod[idx];
            if (fval > dval) {
                accumulator += 1;
            }
        }
    }
    
    printf("Final accumulator: %d\n", accumulator);
    
    // Verify some results
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
