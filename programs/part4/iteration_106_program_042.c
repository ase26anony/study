#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int delta) {
    *ptr += delta;
}

// Another opaque function for aliasing complexity
__attribute__((noinline, noclone))
int* get_pointer(int* base, int idx) {
    // Add some computation to prevent optimization
    return base + (idx ^ 0x55) ^ 0xAA;
}

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 30);
    }
    
    // Main computational loop - high trip count encourages if-conversion
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with some computation
        int idx = (outer * 13) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access to both arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test_expr
            
            // 1. Direct modification of data[idx] - part of test_expr
            data[idx] += 1;
            
            // 2. Function call with address of mod[idx] - volatile part of test_expr
            modify_value(&mod[idx], 2);
            
            // 3. Assignment that creates aliasing complexity
            // Use pointer that might alias with test expression
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = (*alias_ptr * 3) / 2;
            
            // 4. Additional arithmetic to create more RTL instructions
            int temp = mod[idx];
            mod[idx] = temp - (data[idx] % 10);
            
            // 5. Another potential modification
            if (idx > 0) {
                data[idx-1] = data[idx] / 2;
            }
            
            accumulator += data[idx] * 2;
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = data[idx] + 5;
            accumulator -= mod[idx];
        }
        
        // Additional computation to prevent dead code elimination
        if (outer % 7 == 0) {
            // Create another conditional inside the loop
            // to increase basic block complexity
            int alt_idx = (idx + 1) % SIZE;
            if (data[alt_idx] < mod[alt_idx]) {
                data[alt_idx] = mod[alt_idx] - data[alt_idx];
            }
        }
        
        // Mix data types for expression complexity
        unsigned int unsigned_check = (unsigned int)data[idx];
        if (unsigned_check > 100U) {
            accumulator += 1;
        }
    }
    
    printf("Final accumulator: %d\n", accumulator);
    
    // Verify some results to prevent complete optimization
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
