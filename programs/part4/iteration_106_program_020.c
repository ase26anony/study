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

int main() {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    
    // Initialize arrays with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with complex expression
        int idx = (outer * 17 + 23) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses memory accesses with mixed types
        if (data[idx] > (int)mod[idx]) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] used in condition
            data[idx] = data[idx] - 1;
            
            // 2. Function call that takes address of mod[idx]
            // This creates aliasing concerns for test_expr analysis
            modify_value(&mod[idx]);
            
            // 3. Arithmetic operation that could be seen as modifying
            // the memory locations used in the test expression
            int temp = compute_threshold(data[idx]);
            
            // 4. Assignment that uses both sides of original condition
            mod[idx] = data[idx] / 2 + temp % 3;
            
            // 5. Additional operations to create more RTL instructions
            for (int j = 0; j < 3; ++j) {
                data[idx] += j;
                mod[idx] -= j;
            }
            
            // 6. Final operation that might alias
            if (mod[idx] < 100) {
                data[idx] = mod[idx] * 2;
            }
        } else {
            // ELSE BLOCK: Different operations to ensure both paths are live
            mod[idx] = data[idx] + 5;
            data[idx] = compute_threshold(mod[idx]);
        }
        
        // Additional computation to prevent dead code elimination
        accumulator += data[idx] - mod[idx];
        
        // Complex loop-carried dependency
        if (outer % 100 == 0) {
            // This creates additional control flow
            for (int k = 0; k < 5; ++k) {
                data[(idx + k) % SIZE] += accumulator % 100;
            }
        }
    }
    
    printf("Final accumulator: %d\n", accumulator);
    
    // Verify some values to prevent complete optimization
    int sum = 0;
    for (int i = 0; i < SIZE; ++i) {
        sum += data[i];
    }
    printf("Data sum: %d\n", sum);
    
    return 0;
}
