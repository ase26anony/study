#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int delta) {
    *ptr += delta;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int x, int y) {
    return (x * 3 + y) / 2;
}

int main(void) {
    // Arrays with different storage classes
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = (i * 7) % 150;
    }
    
    volatile int accumulator = 0;
    volatile int control = 42;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; outer++) {
        // Calculate index with complex expression
        int idx = (outer * 13 + control) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and arithmetic
        if (data[idx] > mod[idx] + compute_threshold(idx, outer)) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] = data[idx] - 1;
            
            // 2. Function call that takes address of mod[idx]
            modify_value(&mod[idx], idx % 10);
            
            // 3. Assignment that could alias condition memory
            int temp = data[idx] * 2;
            mod[idx] = temp / 3;  // This modifies mod[idx] from test expression
            
            // 4. More arithmetic operations
            for (int j = 0; j < 3; j++) {
                data[idx] += j;
            }
            
            // 5. Complex expression involving the same memory
            control = (control + data[idx] - mod[idx]) % 100;
            
        } else {
            // ELSE BLOCK: Different operations to keep path live
            mod[idx] = mod[idx] * 2;
            data[idx] = data[idx] + 5;
            control = (control - 1) % 100;
        }
        
        // Additional operations to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Nested condition to create more basic blocks
        if (accumulator > 1000000) {
            accumulator = accumulator % 1000000;
            modify_value(&control, 1);
        }
        
        // Loop-carried dependency
        if (outer % 100 == 0) {
            int temp_idx = (outer / 100) % SIZE;
            data[temp_idx] = compute_threshold(data[temp_idx], mod[temp_idx]);
        }
    }
    
    printf("Result: accumulator = %d, control = %d\n", accumulator, control);
    
    // Verify some values to prevent complete optimization
    int verify_sum = 0;
    for (int i = 0; i < SIZE; i++) {
        verify_sum += data[i];
    }
    printf("Data sum: %d\n", verify_sum);
    
    return 0;
}
