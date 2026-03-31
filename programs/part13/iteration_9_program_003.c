#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 8
#define MIDDLE_SIZE 16

static int function_a(int *arr, int n) {
    int sum = 0;
    // Triple-nested loop with fully contained sub-loops
    for (int i = 0; i < n; i++) {
        // First inner loop - fully contained
        for (int j = 0; j < INNER_SIZE; j++) {
            sum += arr[i * INNER_SIZE + j];
            // Multiple exit points
            if (sum > 1000000) break;
        }
        
        // Second inner loop - also fully contained
        int k = 0;
        while (k < MIDDLE_SIZE) {
            sum -= arr[i * MIDDLE_SIZE + k];
            k++;
            // Another exit point
            if (sum < -500000) break;
        }
        
        // Loop-invariant code to encourage strength reduction analysis
        int stride = 4;  // Loop invariant
        for (int m = 0; m < 4; m++) {
            sum += arr[i * stride + m] * 2;
        }
    }
    return sum;
}

static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    // Two loops that will share a common basic block
    // First loop
    for (i = 0; i < n/2; i++) {
        result += arr[i];
        if (result % 7 == 0) {
            goto common_block;  // Creates overlapping control flow
        }
    }
    
    // Second loop - partially overlaps with first via common_block
    while (i < n) {
        result -= arr[i];
        i++;
        
        if (result % 11 == 0) {
            goto common_block;  // Same common block
        }
    }
    
    return result;
    
common_block:
    // Common basic block shared by both loops
    result *= 2;
    // Jump back differently based on context
    if (i < n/2) {
        goto continue_first_loop;
    } else {
        goto continue_second_loop;
    }
    
continue_first_loop:
    // Continue first loop
    i++;
    if (i < n/2) {
        result += arr[i] * 3;
        goto common_block;  // Creates irreducible flow
    }
    return result;
    
continue_second_loop:
    // Continue second loop  
    if (i < n) {
        result -= arr[i] / 2;
        i++;
        goto common_block;  // More irreducible flow
    }
    return result;
}

static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    // Loop with switch containing break to outside
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        switch (state) {
            case 0:
                if (total > 1000) {
                    state = 1;
                }
                break;
            case 1:
                if (total > 5000) {
                    // This break exits the switch, not the loop
                    break;
                }
                // Complex control: break to label outside loop
                if (arr[i] % 13 == 0) {
                    goto exit_loop;
                }
                break;
            case 2:
                total *= 2;
                // Conditional break from loop
                if (total > 10000) {
                    goto exit_loop;
                }
                break;
        }
        
        // Multiple basic blocks within loop
        if (i % 3 == 0) {
            total += 1;
        } else if (i % 3 == 1) {
            total -= 1;
        } else {
            total *= 1;  // No-op but creates another basic block
        }
        
        // Another exit point
        if (total < -10000) break;
        
        continue;
        
    exit_loop:
        // Label inside loop but break target
        total += 100;
        break;  // Breaks the for loop
    }
    
    return total;
}

static int function_d(int *arr, int n) {
    // Adjacent loops that might be analyzed together
    int sum1 = 0, sum2 = 0;
    
    // First adjacent loop
    for (int i = 0; i < n; i += 2) {
        sum1 += arr[i];
        // Early exit creates more basic blocks
        if (sum1 > 5000) break;
    }
    
    // Second adjacent loop - shares no blocks with first
    for (int i = 1; i < n; i += 2) {
        sum2 += arr[i];
        if (sum2 > 5000) break;
    }
    
    // Third loop that uses both sums
    int final = 0;
    for (int i = 0; i < n && i < 100; i++) {
        final += arr[i] + sum1 - sum2;
        // Nested if-else to create internal blocks
        if (final % 2 == 0) {
            final /= 2;
        } else {
            final *= 3;
        }
    }
    
    return final;
}

int main() {
    // Initialize array
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 3 + 7) % 97;  // Semi-random values
    }
    
    // Call functions with different slices
    int result = 0;
    
    // Function A: Triple-nested loops (fully contained sub-loops)
    result += function_a(arr, SIZE / 8);
    
    // Function B: Overlapping loops via goto
    result += function_b(arr + 100, SIZE - 100);
    
    // Function C: Loop with switch and break to label
    result += function_c(arr + 200, SIZE - 200);
    
    // Function D: Adjacent loops
    result += function_d(arr + 300, SIZE - 300);
    
    // Use volatile to prevent elimination
    volatile int sink = result;
    
    // Also print to ensure code runs
    printf("Result: %d\n", result);
    
    return result % 256;
}
