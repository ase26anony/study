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
    return (base * idx) % 256;
}

// Complex computation to create multi-instruction basic block
__attribute__((noinline, noclone))
int complex_transform(int x, volatile int* mod) {
    int temp = x * 3;
    temp = temp ^ 0x55AA55AA;
    *mod = temp >> 4;  // This modifies memory that might be part of test_expr
    return temp;
}

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 3) % 100;
    }
    
    // Main computational loop - high trip count encourages if-conversion
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 31) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory accesses and arithmetic
        if (data[idx] > mod[idx] + compute_threshold(outer, idx)) {
            // THEN BLOCK: Multiple statements that could modify test_expr
            // This creates several RTL instructions before block boundary
            
            // 1. Direct modification of data[idx] - part of test_expr
            data[idx] += 2;
            
            // 2. Function call that takes address of mod[idx] 
            // (could alias with test_expr)
            modify_value(&mod[idx], 1);
            
            // 3. Complex operation that might be seen as modifying test_expr
            int temp = complex_transform(data[idx], &mod[idx]);
            
            // 4. Assignment that uses both sides of original condition
            mod[idx] = data[idx] / 2 + (temp % 10);
            
            // 5. Additional arithmetic to extend basic block
            data[idx] = (data[idx] * 3 - mod[idx]) & 0xFF;
            
        } else {
            // ELSE BLOCK: Different operations to ensure both paths are live
            mod[idx] -= 3;
            data[idx] = (data[idx] + mod[idx]) % 256;
            
            // Additional else-block operations
            if (mod[idx] < 0) {
                mod[idx] = 0;
            }
        }
        
        // Use results to prevent dead code elimination
        accumulator += data[idx] - mod[idx];
        
        // Additional loop operations to create rich CFG
        if (outer % 100 == 0) {
            // Nested condition inside loop
            int alt_idx = (idx + 1) % SIZE;
            if (data[alt_idx] > mod[alt_idx]) {
                data[alt_idx] >>= 1;
            } else {
                mod[alt_idx] <<= 1;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
