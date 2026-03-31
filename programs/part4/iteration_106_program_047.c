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
    return (base ^ idx) & 0x7F;
}

// Global volatile to force memory operations
volatile int global_counter = 0;

#define ARRAY_SIZE 1024

int main() {
    // Arrays with different storage classes
    int data[ARRAY_SIZE];
    volatile int mod[ARRAY_SIZE];
    int temp_buffer[ARRAY_SIZE];
    
    // Initialize with pattern
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
        temp_buffer[i] = 0;
    }
    
    volatile int accumulator = 0;
    int outer_iterations = 1000000;
    
    // Main computational loop - designed for if-conversion analysis
    for (int iter = 0; iter < outer_iterations; iter++) {
        // Calculate index with complex expression
        int idx = (iter * 17 + 23) % ARRAY_SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses memory accesses with volatile
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] = data[idx] - mod[idx];
            
            // 2. Function call that takes address of mod[idx]
            modify_value(&mod[idx], data[idx] % 10);
            
            // 3. Complex computation that might alias
            int temp = compute_threshold(data[idx], idx);
            
            // 4. Assignment that modifies mod[idx] - part of original test
            mod[idx] = temp + (data[idx] >> 1);
            
            // 5. Additional arithmetic
            temp_buffer[idx] = (temp_buffer[idx] + 1) % 100;
            
            // 6. Another memory access pattern
            if (idx > 0) {
                data[idx-1] = (data[idx-1] + temp) & 0xFF;
            }
            
        } else {
            // ELSE BLOCK: Different operations
            int diff = mod[idx] - data[idx];
            data[idx] = data[idx] + (diff / 3);
            mod[idx] = mod[idx] - (diff % 5);
            temp_buffer[idx] = (temp_buffer[idx] * 2) % 100;
        }
        
        // Additional computation to prevent dead code elimination
        int result = data[idx] + mod[idx] + temp_buffer[idx];
        accumulator += result;
        
        // Complex loop-carried dependency
        if (iter % 100 == 0) {
            global_counter += accumulator % 1000;
        }
    }
    
    // Use results to prevent optimization
    printf("Final accumulator: %d\n", accumulator);
    printf("Global counter: %d\n", global_counter);
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
