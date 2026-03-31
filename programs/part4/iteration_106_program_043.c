#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr) {
    *ptr += 1;
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int x) {
    return (x * 3) / 2;
}

// Complex computation to create multiple basic blocks
__attribute__((noinline, noclone))
int complex_update(int a, int b, volatile int* mod) {
    int temp = a + b;
    *mod = temp % 100;  // This modifies volatile memory
    return temp * 2;
}

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];  // volatile to prevent optimization
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = (i * 7) % 100;
    }
    
    // Main computational loop - high trip count encourages if-conversion
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + outer / 3) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression involves memory access to both arrays
        if (data[idx] > mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test_expr
            
            // 1. Direct modification of data[idx] - could affect test_expr
            data[idx] += 2;
            
            // 2. Function call with side effects on mod[idx]
            modify_value(&mod[idx]);
            
            // 3. Complex computation that might alias
            int temp = complex_update(data[idx], mod[idx], &mod[idx]);
            
            // 4. Assignment that could be seen as modifying test_expr
            // This creates multiple RTL instructions
            mod[idx] = data[idx] / 2 + temp % 3;
            
            // 5. Additional arithmetic to extend basic block
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
            }
            
            // 6. Another volatile access
            volatile int* vptr = &mod[idx];
            *vptr = (*vptr + 1) % 100;
            
        } else {
            // ELSE BLOCK: Different operations to keep path alive
            mod[idx] = compute_threshold(data[idx]);
            data[idx] -= 1;
            
            // Additional else-block operations
            if (mod[idx] < 50) {
                mod[idx] *= 2;
            }
        }
        
        // Additional computation using results to prevent dead code elimination
        accumulator += data[idx] + mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 100 == 0) {
            // This creates control flow within the loop
            int alt_idx = (idx + 1) % SIZE;
            if (data[alt_idx] > 0) {
                mod[alt_idx] = data[alt_idx] - 1;
            }
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
