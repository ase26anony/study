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
    return (base + idx * 3) % 100;
}

// Function to create complex then-block with potential modification
__attribute__((noinline, noclone))
void process_then_block(volatile int* mod, int* data, int idx, int* counter) {
    // Multiple operations that could be seen as modifying test expression
    int temp = data[idx];
    
    // First operation - direct modification
    data[idx] = temp + 1;
    
    // Function call with address of volatile
    modify_value(&mod[idx], 2);
    
    // Complex arithmetic that might alias
    int computed = (mod[idx] * 3) / 2;
    
    // Assignment back to data array
    data[idx] = computed - temp;
    
    // Another operation on the same memory
    mod[idx] = data[idx] % 50;
    
    // Increment counter to prevent dead code elimination
    (*counter)++;
}

int main(void) {
    const int SIZE = 1024;
    const int OUTER_ITERATIONS = 1000000;
    
    // Arrays with different storage classes
    int data[SIZE];
    volatile int mod[SIZE];
    volatile int accumulator = 0;
    volatile int control_var = 42;
    
    // Initialize with pattern
    for (int i = 0; i < SIZE; i++) {
        data[i] = i % 100;
        mod[i] = (i * 7) % 100;
    }
    
    // Outer loop to encourage if-conversion
    for (int outer = 0; outer < OUTER_ITERATIONS; outer++) {
        // Calculate index with complex expression
        int idx = (outer * 13 + control_var) % SIZE;
        
        // CRITICAL IF-THEN-ELSE STRUCTURE
        // Test expression uses memory accesses with volatile
        if (data[idx] > mod[idx] && (outer % 3) == 0) {
            // THEN BLOCK - designed to trigger the uncovered code
            // Multiple statements creating several RTL instructions
            
            // 1. Direct modification of data[idx] used in condition
            int old_val = data[idx];
            data[idx] = old_val + (mod[idx] % 10);
            
            // 2. Function call that might modify test expression components
            modify_value(&mod[idx], data[idx] % 5);
            
            // 3. Complex operation sequence
            for (int j = 0; j < 3; j++) {
                mod[idx] += j;
                data[idx] -= j * 2;
            }
            
            // 4. Another assignment that could alias
            int threshold = compute_threshold(data[idx], idx);
            if (mod[idx] > threshold) {
                mod[idx] = threshold;
            }
            
            // 5. Final modification
            data[idx] = (data[idx] + mod[idx]) / 2;
            
            // Additional computation to extend basic block
            accumulator += data[idx] - mod[idx];
            
        } else {
            // ELSE BLOCK - simpler but necessary for if-conversion
            int temp = mod[idx];
            mod[idx] = data[idx] + temp;
            data[idx] = temp - 1;
            accumulator -= data[idx];
        }
        
        // Additional loop operations to prevent optimization
        control_var = (control_var + outer) % 1000;
        
        // Memory barrier effect
        asm volatile("" : : : "memory");
    }
    
    // Use results to prevent elimination
    printf("Result: accumulator = %d, control_var = %d\n", 
           accumulator, control_var);
    
    // Verify some values
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += data[i];
    }
    printf("Data array sum: %d\n", sum);
    
    return 0;
}
