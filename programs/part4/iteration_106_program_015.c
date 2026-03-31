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
void process_then_block(volatile int* data, volatile int* mod, int idx, int* counter) {
    // Multiple operations that could be seen as modifying the test expression
    int temp = data[idx];
    temp = temp * 2 + 1;
    
    // Call to opaque function - creates multiple RTL instructions
    modify_value(&mod[idx], temp);
    
    // Arithmetic that might alias
    data[idx] = temp / 3;
    
    // Another function call
    *counter += compute_threshold(data[idx], idx);
    
    // More arithmetic
    mod[idx] = (mod[idx] + data[idx]) % 256;
}

int main() {
    const int SIZE = 1024;
    volatile int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 7) % 100;
    }
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 100000; ++outer) {
        // Inner loop containing the target if-then-else structure
        for (int i = 0; i < SIZE - 1; ++i) {
            // Complex index calculation
            int idx = (i + outer) % SIZE;
            
            // CRITICAL: Test expression using memory accesses with mixed types
            // The volatile qualifier prevents hoisting
            int test_val = data[idx];
            int mod_val = mod[idx];
            
            // The target if-then-else structure
            if (test_val > mod_val) {
                // THEN BLOCK: Multiple statements that create several RTL instructions
                // This should trigger the scan in ifcvt.cc
                
                // 1. Modify data[idx] - could affect test expression
                data[idx] = test_val - mod_val;
                
                // 2. Function call with side effects
                modify_value(&mod[idx], data[idx]);
                
                // 3. More arithmetic operations
                int temp = data[idx] * 2;
                mod[idx] = temp % 100;
                
                // 4. Another modification that might alias
                data[idx + 1] = data[idx] + mod[idx];
                
                // 5. Call to complex function
                int local_counter = 0;
                process_then_block(data, mod, idx, &local_counter);
                accumulator += local_counter;
                
                // 6. Final assignment
                mod[idx] = data[idx] / 2 + 1;
                
            } else {
                // ELSE BLOCK: Different operations to keep path alive
                mod[idx] = test_val + mod_val;
                data[idx] = mod[idx] % 50;
                accumulator -= 1;
            }
            
            // Additional operations to prevent dead code elimination
            accumulator += data[idx] - mod[idx];
            
            // Complex loop-carried dependency
            if (idx % 3 == 0) {
                data[(idx + 1) % SIZE] = accumulator % 1000;
            }
        }
        
        // Loop variant computation
        mod[outer % SIZE] = accumulator % 256;
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
