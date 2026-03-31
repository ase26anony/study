#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 8
#define MID_SIZE 16

static int function_a(int* arr, int n) {
    int sum = 0;
    
    // Triple nested loop - fully contained sub-loops
    // Outer loop
    for (int i = 0; i < n; i += MID_SIZE) {
        // Middle loop - fully contained within outer
        for (int j = 0; j < MID_SIZE && (i + j) < n; j++) {
            // Inner loop - fully contained within middle
            for (int k = 0; k < INNER_SIZE; k++) {
                sum += arr[i + j] * k;
            }
            
            // Multiple exit points
            if (sum > 1000000) {
                break;
            }
        }
        
        // Loop-invariant code
        int stride = 2;
        for (int j = 0; j < MID_SIZE && (i + j) < n; j++) {
            sum += arr[i + j * stride];
        }
    }
    
    return sum;
}

static int function_b(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    // Two loops that will share a common basic block
    // First loop
    while (i < n / 2) {
        result += arr[i] * 3;
        i++;
        
        // Conditional break creates additional basic block
        if (result > 50000) {
            goto common_block;
        }
    }
    
    // Reset for second loop
    i = n / 2;
    
    // Second loop - partially overlaps with first in control flow
    while (i < n) {
        result -= arr[i] * 2;
        i++;
        
        if (result < -10000) {
            goto common_block;
        }
    }
    
    // This is the common block that both loops can jump to
    // Creates partial overlap in block bitmaps
common_block:
    result = result * 2 + 1;
    
    // Another loop that shares the common_block via different path
    for (int j = 0; j < n; j += 4) {
        result += arr[j];
        
        // This creates a scenario where loops aren't proper subsets
        if (j % 8 == 0) {
            goto common_block;
        }
    }
    
    return result;
}

static int function_c(int* arr, int n) {
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
                // This break exits the switch, not the loop
                if (arr[i] % 7 == 0) {
                    // Complex control flow: goto inside switch inside loop
                    goto special_handler;
                }
                break;
            case 2:
                // Direct break from loop from within switch
                if (total > 5000) {
                    goto loop_exit;
                }
                break;
        }
        
        // Multiple basic blocks within loop
        if (i % 3 == 0) {
            total += 1;
        } else if (i % 3 == 1) {
            total -= 1;
        } else {
            total *= 2;
        }
        
        continue;
        
special_handler:
        total += 100;
        state = 2;
    }
    
loop_exit:
    
    // Another loop with irreducible control flow
    int j = 0;
    while (j < n) {
        total += arr[j];
        j += 2;
        
        // Nested loop inside while
        for (int k = 0; k < 3 && (j + k) < n; k++) {
            total -= arr[j + k];
            
            // Early exit from inner loop
            if (total < 0) {
                break;
            }
        }
    }
    
    return total;
}

static int function_d(int* arr, int n) {
    int acc = 0;
    
    // Complex loop nest with sibling loops
    // First sibling loop
    for (int i = 0; i < n; i++) {
        acc += arr[i];
        
        // Early exit creates additional basic block
        if (acc > 10000) {
            goto shared_code;
        }
    }
    
    // Second sibling loop - not nested, but shares code
    for (int i = n - 1; i >= 0; i--) {
        acc -= arr[i];
        
        if (acc < -5000) {
            goto shared_code;
        }
    }
    
    return acc;

shared_code:
    // This block is shared by both loops above
    // Creates partial overlap scenario
    acc = abs(acc);
    
    // Yet another loop that contains the shared_code as part of its body
    // but also has blocks outside of it
    for (int i = 0; i < n; i += 2) {
        acc += arr[i] * 2;
        
        if (i % 4 == 0) {
            // Jump back to shared_code - creates cycle
            goto shared_code;
        }
    }
    
    return acc;
}

int main() {
    // Initialize array with sequential values
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    // Call functions with different slices to create varied loop structures
    int result1 = function_a(arr, SIZE);
    int result2 = function_b(arr + SIZE/4, SIZE/2);
    int result3 = function_c(arr + SIZE/2, SIZE/4);
    int result4 = function_d(arr, SIZE);
    
    // Combine results to prevent elimination
    int final_result = result1 + result2 + result3 + result4;
    
    // Use volatile to ensure computation isn't optimized away
    volatile int sink = final_result;
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}
