#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

// Simple pseudo-random generator to create data dependencies
static inline unsigned int lcg(unsigned int *state) {
    return *state = (*state * 1103515245 + 12345) & 0x7fffffff;
}

// Function containing the target loop
int process_array(volatile int *arr, int n, volatile int init) {
    volatile int sum = init;
    volatile int prev = 0;
    volatile int curr = arr[0];
    
    // Outer loop to provide context
    for (int outer = 0; outer < 3; outer++) {
        // Reset for each outer iteration
        sum = init + outer;
        prev = 0;
        curr = arr[0];
        
        // Target inner loop with loop-carried dependencies
        for (int i = 1; i < n; i++) {
            // Multiple arithmetic operations creating dependencies
            int temp1 = arr[i] * 3;      // Multiplication
            int temp2 = temp1 >> 2;      // Shift operation
            int temp3 = temp2 + prev;    // Uses value from previous iteration
            
            // Recurrence: current depends on previous iteration
            prev = curr;                 // distance1_use: prev used next iteration
            curr = temp3 + i;            // Complex calculation
            
            // Accumulator with loop-carried dependency
            sum = sum + curr * sum;      // sum_i depends on sum_{i-1}
            
            // Additional operations to increase scheduling complexity
            int temp4 = (sum & 0xFF) * 7;
            prev = prev ^ temp4;         // Modify prev further
        }
    }
    
    return sum;
}

int main() {
    volatile int array[SIZE];
    volatile unsigned int seed = 42;
    
    // Initialize array with pseudo-random values
    for (int i = 0; i < SIZE; i++) {
        array[i] = lcg(&seed) % 100;
    }
    
    // Volatile initial value to prevent constant propagation
    volatile int init_val = 17;
    
    // Execute the processing function multiple times
    int total = 0;
    for (int repeat = 0; repeat < 5; repeat++) {
        init_val = init_val + repeat;
        int result = process_array(array, SIZE, init_val);
        total += result;
        
        // Modify array slightly between repetitions
        for (int i = 0; i < SIZE; i += 64) {
            array[i] = array[i] + 1;
        }
    }
    
    printf("Final result: %d\n", total);
    return 0;
}
