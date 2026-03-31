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
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    volatile int accumulator = 0;
    int outer_iterations = 1000000;
    
    // Outer loop to encourage if-conversion analysis
    for (int iter = 0; iter < outer_iterations; iter++) {
        int idx = iter % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access to volatile array
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx]--;
            
            // 2. Function call that takes address of mod[idx] - volatile part of test
            modify_value(&mod[idx], 1);
            
            // 3. Complex assignment that could alias test expression
            int* alias_ptr = get_pointer(data, idx);
            *alias_ptr = (*alias_ptr) * 2 % 100;
            
            // 4. Another operation on mod[idx]
            mod[idx] = data[idx] / 2 + mod[idx] % 3;
            
            // 5. Additional arithmetic to create more RTL instructions
            for (int j = 0; j < 3; j++) {
                mod[idx] += j;
            }
            
        } else {
            // ELSE BLOCK: Different operations
            data[idx] += 2;
            mod[idx] -= 1;
            
            // Additional else-block operations
            int temp = data[idx] * 3;
            if (temp > 100) {
                mod[idx] = temp % 100;
            }
        }
        
        // Prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Additional loop operations to create complex CFG
        if (iter % 100 == 0) {
            // Nested condition to create more basic blocks
            int idx2 = (idx + 1) % SIZE;
            if (data[idx2] < mod[idx2]) {
                data[idx2] = mod[idx2] + data[idx];
                modify_value(&mod[idx2], -1);
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
