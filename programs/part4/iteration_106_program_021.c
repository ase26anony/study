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
    
    volatile long long accumulator = 0;
    
    // Outer loop with high trip count to encourage if-conversion
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index - use complex expression to prevent hoisting
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access to volatile and non-volatile arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] += 2;
            
            // 2. Function call that takes address of mod[idx] - volatile part of test
            modify_value(&mod[idx], 1);
            
            // 3. Assignment that could alias - using pointer from function
            int* ptr = get_pointer(data, idx);
            *ptr = (*ptr % 50) + 10;
            
            // 4. Additional arithmetic to create more instructions
            int temp = mod[idx];
            mod[idx] = temp + data[idx] / 3;
            
            // 5. Another potential modification through different expression
            if (idx > 0) {
                data[idx-1] = data[idx] - 5;
            }
            
            // Final computation in then block
            accumulator += data[idx] * 3LL;
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] -= 1;
            data[idx] = data[idx] * 2 % 100;
            accumulator -= mod[idx];
        }
        
        // Additional operations to prevent dead code elimination
        if (outer % 7 == 0) {
            int alt_idx = (idx + 1) % SIZE;
            if (data[alt_idx] < mod[alt_idx]) {
                data[alt_idx] += mod[alt_idx];
            }
        }
    }
    
    printf("Result: %lld\n", accumulator);
    return 0;
}
