#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 16
#define MID_SIZE 32

static int function_a(int* arr, int n) {
    int sum = 0;
    
    // Triple nested loop with fully contained inner loops
    // This creates proper sub-loop relationships
    for (int i = 0; i < n; i += 4) {
        // Outer loop body starts
        int outer_acc = arr[i];
        
        for (int j = 0; j < MID_SIZE; j++) {
            // Middle loop body
            int mid_acc = 0;
            
            for (int k = 0; k < INNER_SIZE; k++) {
                // Innermost loop - fully contained in middle loop
                mid_acc += arr[(i + j + k) % n];
                if (mid_acc > 1000) {
                    // Multiple exit point
                    break;
                }
            }
            
            outer_acc += mid_acc;
            if (outer_acc > 5000) {
                // Another exit point
                break;
            }
        }
        
        sum += outer_acc;
    }
    
    return sum;
}

static int function_b(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    // First loop with goto to shared block
    for (i = 0; i < n/2; i++) {
        result += arr[i] * 2;
        
        if (result > 10000) {
            goto shared_block;
        }
        
        // Loop-invariant code
        int stride = 3;  // Loop invariant
        result -= arr[i * stride % n];
    }
    
    // Jump here from first loop
    shared_block:
    
    // Second loop that also uses the shared block
    // These loops partially overlap in control flow
    for (int j = n/2; j < n; j++) {
        result += arr[j] / 2;
        
        if (j % 8 == 0) {
            goto shared_block;  // Jump back to shared block
        }
        
        // Another basic block
        if (result < 0) {
            result = 0;
        }
    }
    
    return result;
}

static int function_c(int* arr, int n) {
    int total = 0;
    int state = 0;
    
    // Loop with switch containing break to exit loop
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        switch (state) {
            case 0:
                if (total > 500) {
                    state = 1;
                }
                break;
            case 1:
                if (arr[i] % 7 == 0) {
                    // This break exits the switch, not the loop
                    break;
                }
                // Fall through
            case 2:
                if (total > 1000) {
                    // This goto creates complex control flow
                    goto loop_exit;
                }
                break;
            default:
                // Direct break from switch to outside loop
                goto loop_exit;
        }
        
        // Loop-invariant computation
        int invariant = n / 4;
        total -= arr[i % invariant];
    }
    
    loop_exit:
    return total;
}

static int function_d(int* arr, int n) {
    int acc = 0;
    
    // Two adjacent loops that might be analyzed as overlapping
    // due to compiler's basic block analysis
    for (int i = 0; i < n; i += 2) {
        acc += arr[i];
        
        // Conditional break creates multiple basic blocks
        if (acc > 2000) {
            break;
        }
        
        // Strength reduction candidate
        acc += arr[i * 3 % n];
    }
    
    // This loop starts where previous might have ended
    // creating potential bitmap intersection
    int start = (n / 3);
    while (start < n) {
        acc -= arr[start];
        
        // Multiple exit points
        if (acc < -1000) {
            break;
        }
        
        // Another basic block
        for (int j = 0; j < 4; j++) {
            acc += j;
            if (j == 2) {
                // Nested break
                break;
            }
        }
        
        start += (n / 4);
    }
    
    return acc;
}

int main(int argc, char** argv) {
    // Initialize array with sequential values
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i + 1;
    }
    
    // Call functions with different loop structures
    int result = 0;
    
    // Function A: Triple nested loops (fully contained sub-loops)
    result += function_a(arr, SIZE);
    
    // Function B: Overlapping loops with goto
    result += function_b(arr, SIZE);
    
    // Function C: Loop with switch and complex break
    result += function_c(arr, SIZE);
    
    // Function D: Adjacent loops with potential overlap
    result += function_d(arr, SIZE);
    
    // Use result to prevent elimination
    printf("Result: %d\n", result);
    
    // Also return from main to ensure all code is needed
    return result % 256;
}
