#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define OUTER_ITERATIONS 1000000

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr) {
    *ptr += 1;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int base) {
    return (base * 3) / 2;
}

int main() {
    // Arrays with different storage classes
    int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    // Main computational loop
    for (int outer = 0; outer < OUTER_ITERATIONS; outer++) {
        // Calculate index - complex enough to prevent hoisting
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access and volatile
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test_expr
            // 1. Direct modification of data[idx] used in condition
            data[idx] = data[idx] - compute_threshold(mod[idx]);
            
            // 2. Function call with address of volatile mod[idx]
            // This creates aliasing concerns
            modify_value(&mod[idx]);
            
            // 3. Assignment that could alias with test expression
            // Using same memory locations as condition
            int temp = data[idx];
            mod[idx] = temp / 2;
            
            // 4. Additional arithmetic to create more RTL instructions
            for (int j = 0; j < 3; j++) {
                data[idx] += j;
            }
            
            // 5. Another volatile access
            accumulator += mod[idx];
        } else {
            // ELSE BLOCK: Different operations to keep both paths live
            mod[idx] = data[idx] * 2;
            data[idx] += mod[idx] % 7;
            accumulator -= data[idx];
        }
        
        // Additional computation to prevent dead code elimination
        if (outer % 7 == 0) {
            int* ptr = &data[idx];
            *ptr += outer % 11;
        }
    }
    
    // Use results to prevent optimization
    printf("Final accumulator: %d\n", accumulator);
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
