#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int value) {
    *ptr = value;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int a, int b) {
    return (a + b) / 3;
}

// Function to create complex control flow
__attribute__((noinline, noclone))
int process_block(int* data, volatile int* mod, int idx, int threshold) {
    int result = 0;
    
    // Complex if-then-else structure with multiple basic blocks
    if (data[idx] > mod[idx]) {
        // THEN BLOCK - This should generate multiple RTL instructions
        // that will be scanned by ifcvt
        
        // 1. Modify data[idx] - could affect test expression
        data[idx] += 2;
        
        // 2. Call function with address of mod[idx]
        modify_value(&mod[idx], data[idx] * 2);
        
        // 3. Complex arithmetic that might alias
        int temp = data[idx] + mod[idx];
        
        // 4. Another modification that could affect test expression
        mod[idx] = data[idx] / 2 + temp % 7;
        
        // 5. More operations to extend basic block
        for (int j = 0; j < 3; j++) {
            data[idx] += j;
        }
        
        // 6. Function call that takes address
        threshold = compute_threshold(data[idx], mod[idx]);
        
        // 7. Final assignment that might modify test expression components
        if (threshold > 100) {
            mod[idx] = threshold - data[idx];
        }
        
        result = 1;
    } else {
        // ELSE BLOCK - simpler path
        data[idx] -= 1;
        mod[idx] += 1;
        result = -1;
    }
    
    return result;
}

int main() {
    // Initialize arrays with pattern
    int data[SIZE];
    volatile int mod[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 150;
    }
    
    volatile int accumulator = 0;
    
    // Outer loop with high trip count
    for (int outer = 0; outer < ITERATIONS; outer++) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // Mixed-type comparison in condition
        unsigned int threshold = (unsigned int)mod[idx] + 50U;
        
        // Complex if-then-else that should trigger if-conversion analysis
        if ((int)data[idx] > (int)threshold) {
            // THEN PATH - multiple statements that might modify test expression
            
            // 1. Direct modification of data used in condition
            data[idx] += outer % 5;
            
            // 2. Call to opaque function with address
            modify_value(&mod[idx], data[idx] + threshold);
            
            // 3. Assignment that could alias condition memory
            int* alias_ptr = &data[idx];
            *alias_ptr = *alias_ptr - (mod[idx] % 10);
            
            // 4. More arithmetic
            mod[idx] = data[idx] * 2 - threshold / 3;
            
            // 5. Nested condition to create more basic blocks
            if (mod[idx] > 1000) {
                data[idx] = 1000;
            }
            
            accumulator += data[idx];
        } else {
            // ELSE PATH
            data[idx] = threshold % 50;
            accumulator -= mod[idx];
        }
        
        // Additional processing to prevent dead code elimination
        int temp_result = process_block(data, mod, idx, threshold);
        accumulator += temp_result;
        
        // Complex index calculation for next iteration
        if (outer % 100 == 0) {
            // This creates additional control flow
            for (int k = 0; k < 5; k++) {
                mod[(idx + k) % SIZE] = data[idx] + k;
            }
        }
    }
    
    printf("Final accumulator: %d\n", accumulator);
    
    // Verify some results
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
