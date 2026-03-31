#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr, int delta) {
    *ptr += delta;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int x, int y) {
    return (x * 3 + y * 7) % 100;
}

// Complex conditional test with mixed operations
__attribute__((noinline, noclone))
int complex_condition(int a, volatile int b, int* c) {
    return (a > b) && (*c != 0) && ((a + b) % 2 == 0);
}

int main(void) {
    // Arrays with different storage classes
    static int data[2048];
    volatile int mod[2048];
    volatile int accum = 0;
    
    // Initialize with pattern
    for (int i = 0; i < 2048; i++) {
        data[i] = i % 256;
        mod[i] = (i * 3) % 256;
    }
    
    // Main computational loop - high trip count
    for (int outer = 0; outer < 1000000; outer++) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 31) % 2048;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses multiple memory accesses with volatile
        if (data[idx] > mod[idx] && complex_condition(data[idx], mod[idx], &data[(idx + 1) % 2048])) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of memory used in condition
            data[idx] -= 5;  // This modifies data[idx] used in condition
            
            // 2. Function call with pointer to volatile memory
            modify_value(&mod[idx], 2);  // This modifies mod[idx] used in condition
            
            // 3. Complex computation with potential aliasing
            int temp = compute_threshold(data[idx], mod[idx]);
            
            // 4. Assignment that could affect condition
            if (temp > 100) {
                mod[idx] = data[idx] / 2;  // Another modification
            }
            
            // 5. More operations to create multiple basic blocks
            for (int j = 0; j < 3; j++) {
                data[(idx + j) % 2048] += j;
            }
            
            // 6. Final operation in then block
            accum += data[idx] * 2;
            
        } else {
            // ELSE BLOCK: Different operations
            data[idx] += 3;
            mod[idx] -= 1;
            accum -= mod[idx];
        }
        
        // Additional computation to prevent dead code elimination
        int secondary_idx = (idx * 7) % 2048;
        if (data[secondary_idx] < mod[secondary_idx]) {
            data[secondary_idx] = compute_threshold(data[secondary_idx], mod[secondary_idx]);
        }
        
        // Mix data types in computations
        float fval = data[idx] * 0.5f;
        if (fval > 50.0f) {
            mod[idx] = (int)fval;
        }
        
        // Pointer arithmetic that might confuse analysis
        int* ptr1 = &data[idx];
        volatile int* ptr2 = &mod[idx];
        if (ptr1 != (int*)ptr2) {
            *ptr1 = (*ptr1 + *ptr2) / 2;
        }
    }
    
    // Final computation and output
    int final_sum = 0;
    for (int i = 0; i < 2048; i++) {
        final_sum += data[i] + mod[i];
    }
    
    printf("Result: accum=%d, sum=%d\n", accum, final_sum);
    return 0;
}
