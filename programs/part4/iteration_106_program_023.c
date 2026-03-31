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

// Complex test expression helper
__attribute__((noinline, noclone))
int load_and_transform(volatile int* arr, int idx) {
    return arr[idx] * 2 + 1;
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
        // Complex test expression involving:
        // 1. Memory access (data[idx])
        // 2. Volatile memory access (mod[idx])
        // 3. Function call in condition
        // 4. Mixed operations
        if (data[idx] > load_and_transform(mod, idx) - 10) {
            // THEN BLOCK: Multiple statements that could modify test expression
            
            // 1. Direct modification of data[idx] - part of test expression
            data[idx] += 2;
            
            // 2. Function call that takes address of mod[idx] - could alias
            modify_value(&mod[idx], 1);
            
            // 3. Assignment through pointer that might alias
            int* ptr = get_pointer(data, idx);
            *ptr = (*ptr % 100) + 5;
            
            // 4. Complex arithmetic that uses same memory locations
            mod[idx] = data[idx] / 2 + mod[idx % 16];
            
            // 5. Another operation on test expression component
            if (idx > 0) {
                data[idx] -= data[idx-1] / 10;
            }
            
            // 6. Final assignment that definitely modifies test expression component
            data[idx] = (mod[idx] * 3) / 2;
            
        } else {
            // ELSE BLOCK: Different operations to keep path live
            mod[idx] = data[idx] * 3;
            data[idx] = (mod[idx] + 7) % 100;
        }
        
        // Additional operations to prevent dead code elimination
        accumulator += data[idx] - mod[idx];
        
        // Complex index calculation for next iteration
        idx = (idx * 13 + 7) % SIZE;
        
        // Nested conditional to create more basic blocks
        if (accumulator > 1000) {
            accumulator %= 1000;
            modify_value(&mod[idx], accumulator);
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
