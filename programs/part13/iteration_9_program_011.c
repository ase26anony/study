#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 8
#define MID_SIZE 16

static int function_a(int* arr, int n) {
    int sum = 0;
    // Triple-nested loop with fully contained inner loops
    for (int i = 0; i < n; i++) {
        // Outer loop with multiple basic blocks
        if (arr[i] < 0) break;  // Multiple exit point
        
        for (int j = 0; j < MID_SIZE; j++) {
            // Middle loop with invariant calculation
            int stride = 4;  // Loop-invariant
            for (int k = 0; k < INNER_SIZE; k++) {
                // Innermost fully contained loop
                sum += arr[i] * stride + arr[k];
                if (sum > 1000000) break;  // Another exit point
            }
            // Strength reduction candidate: k * stride
            for (int k = 0; k < INNER_SIZE; k++) {
                sum -= arr[j] * k * stride;
            }
        }
    }
    return sum;
}

static int function_b(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    // First loop with goto to shared block
    while (i < n) {
        result += arr[i];
        i++;
        if (result % 7 == 0) {
            goto shared_block;  // Creates overlapping control flow
        }
    }
    
    i = n / 2;
    // Second loop that also reaches shared_block
    do {
        result -= arr[i];
        i--;
        if (i <= 0) break;
        
        shared_block:
        // Shared basic block between the two loops
        result ^= 0x55;  // Common operation
        // This creates partial overlap in block bitmaps
    } while (i > 0);
    
    return result;
}

static int function_c(int* arr, int n) {
    int total = 0;
    int state = 0;
    
    for (int i = 0; i < n; i++) {
        // Loop with switch containing break to label
        switch (state) {
            case 0:
                total += arr[i];
                state = 1;
                break;
            case 1:
                total -= arr[i] * 2;
                if (total < 0) {
                    // This break exits the switch, not the loop
                    break;
                }
                state = 2;
                // Fall through
            case 2:
                total ^= arr[i];
                if (total > 1000) {
                    goto loop_exit;  // Label break exiting the loop
                }
                state = 0;
                break;
        }
        
        // Additional basic block in loop
        total = (total * 13) % 997;
        
        continue;
        
        loop_exit:
        // Label target for break from switch
        break;  // This breaks the for loop
    }
    
    return total;
}

static int function_d(int* arr, int n) {
    int acc = 0;
    
    // Two adjacent loops that might be analyzed as overlapping
    for (int i = 0; i < n/2; i++) {
        acc += arr[i];
        // Complex exit condition
        if (acc > 50000) {
            // Early exit creates another basic block
            break;
        }
    }
    
    // This loop starts where the previous might have left off
    int j = n/2;
    while (j < n) {
        acc -= arr[j];
        j++;
        
        // Nested while inside while - partially overlapping
        int k = 0;
        while (k < 3 && j + k < n) {
            acc ^= arr[j + k];
            k++;
            // This inner loop is fully contained but creates
            // complex bitmap relationships
        }
        
        if (acc < -10000) break;
    }
    
    return acc;
}

int main() {
    // Initialize array with sequential values
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 3) % 97;  // Non-linear but predictable
    }
    
    // Call functions with different slices
    int result = 0;
    
    // Function A: Triple-nested fully contained loops
    result += function_a(arr, SIZE);
    
    // Function B: Overlapping loops with goto
    result += function_b(arr + 100, SIZE - 100);
    
    // Function C: Loop with switch and label break
    result += function_c(arr + 200, SIZE - 200);
    
    // Function D: Adjacent loops with inner while
    result += function_d(arr + 300, SIZE - 300);
    
    // Use volatile to prevent optimization
    volatile int sink = result;
    
    // Also print to ensure code isn't eliminated
    printf("Result: %d\n", result);
    
    return result % 256;
}
