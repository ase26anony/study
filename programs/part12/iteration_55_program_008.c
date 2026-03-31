#include <stdio.h>
#include <stdint.h>

#define SIZE 1024

// Simple pseudo-random generator to create non-constant data
static inline uint32_t lcg(uint32_t *state) {
    *state = *state * 1103515245 + 12345;
    return *state;
}

// Function containing the target loop
int process_array(volatile int *arr, int n) {
    volatile int sum = 0;
    volatile int prev = 0;
    volatile int curr = 1;
    volatile int temp;
    
    // Loop with multiple operations and loop-carried dependencies
    for (int i = 0; i < n; i++) {
        // Array access with volatile to prevent optimization
        temp = arr[i];
        
        // Recurrence 1: sum depends on previous iteration's sum
        // This creates a loop-carried dependency with distance 1
        sum = sum + temp * 3;
        
        // Recurrence 2: prev/curr chain with distance 1 dependency
        // curr from iteration i becomes prev in iteration i+1
        prev = curr;
        curr = temp + (prev >> 2);  // Use prev from previous iteration
        
        // Additional arithmetic operations to increase scheduling complexity
        sum = sum ^ (curr * 7);
        sum = sum + (temp << 1);
        
        // Another distance-1 dependency
        temp = sum - prev;
        sum = temp + (sum & 0xFF);
    }
    
    return sum;
}

// Outer loop to provide context
void outer_loop(volatile int *arr, int outer_iters) {
    volatile int total = 0;
    
    for (int j = 0; j < outer_iters; j++) {
        // Modify array slightly each outer iteration
        for (int i = 0; i < SIZE; i++) {
            arr[i] = arr[i] + (j & 1);
        }
        
        // Call the inner loop function
        total += process_array(arr, SIZE);
    }
    
    // Use volatile to ensure computation isn't optimized away
    volatile int result = total;
    printf("Result: %d\n", result);
}

int main() {
    volatile int array[SIZE];
    uint32_t seed = 42;
    
    // Initialize array with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg(&seed) % 100;
    }
    
    // Execute multiple times to ensure the loop runs
    outer_loop(array, 10);
    
    return 0;
}
