#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 1024

// Simple pseudo-random generator to create non-constant data
static uint32_t simple_rand(uint32_t *seed) {
    *seed = *seed * 1103515245 + 12345;
    return *seed;
}

// Function containing the target loop
uint64_t process_array(volatile int *array, int count) {
    volatile uint64_t accumulator = 1;  // Prevent optimization
    volatile int prev_val = 0;          // For distance-1 dependency
    volatile int curr_val = 0;
    
    // Create a recurrence with distance-1 dependencies
    for (int i = 0; i < count; i++) {
        // Multiple arithmetic operations to create scheduling complexity
        int array_val = array[i];
        
        // Distance-1 dependency chain: prev_val from previous iteration
        int temp1 = prev_val * 3 + 7;
        int temp2 = array_val * temp1;
        
        // Another operation using prev_val
        int temp3 = (prev_val << 2) | (array_val & 0xF);
        
        // Accumulator with loop-carried dependency
        accumulator = accumulator * 17 + temp2;
        accumulator = accumulator ^ (temp3 * 5);
        
        // Update for next iteration's distance-1 dependency
        prev_val = curr_val;
        curr_val = array_val + (accumulator & 0xFF);
        
        // More arithmetic to increase instruction count
        accumulator = accumulator + (array_val << 3);
        accumulator = accumulator - (prev_val * 11);
    }
    
    return accumulator;
}

// Outer loop to provide context
void outer_loop_processor(volatile int *array, int outer_iters, int inner_count) {
    volatile uint64_t total = 0;
    
    for (int iter = 0; iter < outer_iters; iter++) {
        // Modify array slightly each outer iteration
        for (int i = 0; i < inner_count; i++) {
            array[i] = (array[i] * 3 + iter) & 0xFFFF;
        }
        
        // Call the target inner loop function
        total += process_array(array, inner_count);
        
        // Prevent optimization across outer iterations
        asm volatile("" : "+r" (total) : : "memory");
    }
    
    printf("Final total: %lu\n", (unsigned long)total);
}

int main() {
    volatile int array[ARRAY_SIZE];
    uint32_t seed = 42;
    
    // Initialize array with pseudo-random values
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = simple_rand(&seed) & 0xFFFF;
    }
    
    // Execute the loop processor multiple times
    outer_loop_processor(array, 10, ARRAY_SIZE);
    
    // Second test with different parameters
    volatile int array2[512];
    seed = 123;
    for (int i = 0; i < 512; i++) {
        array2[i] = simple_rand(&seed) & 0xFFF;
    }
    
    // Different loop structure
    volatile uint64_t sum = 0;
    volatile int prev = 0;
    volatile int curr = array2[0];
    
    // Another loop with distance-1 dependencies
    for (int i = 0; i < 512; i++) {
        int val = array2[i];
        
        // Complex recurrence: sum depends on previous sum
        sum = sum * 19 + (val * prev);
        
        // Multiple uses of prev (distance-1)
        int t1 = prev + (val >> 2);
        int t2 = (prev * 3) ^ val;
        
        sum = sum + (t1 * t2);
        
        // Update for next iteration
        prev = curr;
        curr = val + (sum & 0xFF);
        
        // Additional arithmetic operations
        sum = sum ^ (prev << 4);
        sum = sum + (curr * 7);
    }
    
    printf("Second sum: %lu\n", (unsigned long)sum);
    
    return 0;
}
