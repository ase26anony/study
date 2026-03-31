#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 16
#define MID_SIZE 32

static int function_a(int* arr, int n) {
    int sum = 0;
    
    // Triple nested loop with fully contained inner loops
    // This should create proper sub-loop relationships
    for (int i = 0; i < n; i += 4) {
        // Outer loop body with multiple basic blocks
        if (arr[i] > 1000) {
            break;  // Creates additional exit point
        }
        
        // Middle loop - fully contained within outer
        for (int j = 0; j < MID_SIZE; j++) {
            // Inner loop - fully contained within middle
            for (int k = 0; k < INNER_SIZE; k++) {
                sum += arr[i] * j * k;
                // Loop-invariant calculation opportunity
                int stride = 4;  // Loop invariant
                sum += arr[(i + j) * stride % n];
            }
            
            // Conditional break in middle loop
            if (sum > 1000000) {
                break;
            }
        }
        
        // Another inner loop at same nesting level
        for (int j = MID_SIZE; j < MID_SIZE * 2; j++) {
            sum -= arr[i + j % n];
        }
    }
    
    return sum;
}

static int function_b(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    // First loop with goto to shared block
    for (i = 0; i < n / 2; i++) {
        result += arr[i] * 2;
        
        // Conditional goto to shared code block
        if (arr[i] % 3 == 0) {
            goto shared_computation;
        }
        
        // Normal loop continuation
        result -= arr[i];
        continue;
        
    shared_computation:
        // Shared basic block between two loops
        result *= 2;
        result += 1;
        // Jump back to loop
        continue;
    }
    
    // Second loop that also uses the shared block
    // This creates partial overlap in basic blocks
    for (int j = n / 2; j < n; j++) {
        result += arr[j] * 3;
        
        if (arr[j] % 4 == 0) {
            goto shared_computation;  // Same label as first loop
        }
        
        result -= arr[j] / 2;
    }
    
    return result;
}

static int function_c(int* arr, int n) {
    int total = 0;
    int state = 0;
    
    // Loop with switch containing break to exit loop
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        // Switch with complex control flow
        switch (state) {
            case 0:
                if (arr[i] < 0) {
                    state = 1;
                }
                total += 1;
                break;
                
            case 1:
                total += arr[i] * 2;
                if (total > 50000) {
                    // This break exits the SWITCH, not the loop
                    break;
                }
                // Fall through
                
            case 2:
                total -= arr[i];
                if (total < -10000) {
                    // This goto exits the entire loop
                    goto loop_exit;
                }
                break;
                
            default:
                // Direct break from switch to loop exit
                if (arr[i] == 0) {
                    goto loop_exit;
                }
        }
        
        // Additional exit point
        if (i > n / 3 && total > 100000) {
            break;
        }
        
        // Nested loop inside case 0 path
        if (state == 0) {
            for (int j = 0; j < 8; j++) {
                total += j * arr[i];
                if (j == 4 && total > 20000) {
                    // Break from inner loop only
                    break;
                }
            }
        }
    }
    
loop_exit:
    return total;
}

static int function_d(int* arr, int n) {
    int acc = 0;
    
    // Two adjacent loops that might be analyzed together
    // First loop with multiple basic blocks
    int i = 0;
    while (i < n) {
        acc += arr[i];
        
        // Conditional with early continue
        if (arr[i] % 5 == 0) {
            i += 2;
            continue;
        }
        
        // Another condition
        if (acc > 5000) {
            i += 3;
        } else {
            i += 1;
        }
        
        // Nested while loop
        int j = 0;
        while (j < 4 && i + j < n) {
            acc -= arr[i + j];
            j++;
            
            // Break from inner while
            if (acc < -1000) {
                break;
            }
        }
    }
    
    // Second while loop - adjacent but separate
    int k = n - 1;
    while (k >= 0) {
        acc += arr[k] * arr[k];
        
        // Complex condition with goto
        if (acc > 100000) {
            goto reduce_acc;
        }
        
        k -= 2;
        continue;
        
    reduce_acc:
        acc /= 2;
        k--;
    }
    
    return acc;
}

int main(int argc, char** argv) {
    // Initialize array with varied values
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i * 3 + 7) % 100 - 50;  // Range -50 to 49
    }
    
    // Force compiler to analyze different loop structures
    int result = 0;
    
    // Call function with triple nested loops (fully contained)
    result += function_a(arr, SIZE);
    
    // Call function with overlapping loops via goto
    result += function_b(arr, SIZE);
    
    // Call function with switch-based control flow
    result += function_c(arr, SIZE);
    
    // Call function with adjacent while loops
    result += function_d(arr, SIZE);
    
    // Use volatile to prevent elimination
    volatile int final_result = result;
    
    // Also print to ensure code executes
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}
