#include <stdio.h>
#include <stdlib.h>

// Opaque function to prevent optimization
__attribute__((noinline, noclone))
void modify_value(volatile int* ptr) {
    *ptr += 1;
    asm volatile("" : : "r"(ptr) : "memory");
}

// Another opaque function
__attribute__((noinline, noclone))
int compute_threshold(int x, int y) {
    return (x * 3 + y * 7) % 100;
}

// Complex conditional test that uses mixed operations
__attribute__((noinline, noclone))
int complex_condition(int a, volatile int b, int* c) {
    // Mixed-type comparison that creates non-trivial RTL
    unsigned int ua = (unsigned int)a;
    int result = (ua > (unsigned int)b) && (*c != 0);
    
    // Memory barrier to prevent reordering
    asm volatile("" : : : "memory");
    return result;
}

int main(void) {
    const int SIZE = 1024;
    int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; ++i) {
        data[i] = i % 100;
        mod[i] = 50 + (i % 50);
    }
    
    // Outer loop with high trip count
    for (int outer = 0; outer < 1000000; ++outer) {
        // Calculate index with wrap-around
        int idx = outer % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Complex test expression involving memory access and function call
        int threshold = compute_threshold(outer, idx);
        
        // This is the test expression that will be analyzed
        if (complex_condition(data[idx], mod[idx], &data[(idx + 1) % SIZE])) {
            // THEN BLOCK - Multiple statements that could modify test expression
            
            // 1. Direct modification of array element used in condition
            data[idx] += 2;  // This modifies data[idx] used in complex_condition
            
            // 2. Function call that takes address of volatile mod[idx]
            modify_value(&mod[idx]);  // This modifies mod[idx] used in condition
            
            // 3. Assignment that creates potential aliasing
            int* alias_ptr = &data[idx];
            *alias_ptr = (*alias_ptr * 3) / 2;
            
            // 4. Additional computation that might be seen as modifying
            // the memory locations used in the original condition
            volatile int temp = mod[idx];
            mod[idx] = data[idx] - threshold;
            
            // 5. More operations to extend basic block
            for (int j = 0; j < 3; ++j) {
                data[(idx + j) % SIZE] += j;
            }
            
            // 6. Final operation that definitely modifies condition component
            data[idx] = (data[idx] > 100) ? 100 : data[idx];
            
        } else {
            // ELSE BLOCK - Different operations to keep path alive
            mod[idx] -= 1;
            data[idx] = (data[idx] * 2) % 200;
            
            // Call to prevent dead code elimination
            modify_value(&mod[(idx + 2) % SIZE]);
        }
        
        // Additional computation using results
        accumulator += data[idx] + mod[idx];
        
        // Prevent loop invariant code motion
        if (outer % 1000 == 0) {
            mod[idx] = accumulator % 1000;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
