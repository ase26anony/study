#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define N 32

static int triple_nested_loop(int *arr, int n) {
    int sum = 0;
    // Function A: Triple nested loop with fully contained inner loops
    for (int i = 0; i < n; i++) {
        // Outer loop body
        int outer_val = arr[i] * 2;
        
        // Middle loop - fully contained within outer
        for (int j = 0; j < i; j++) {
            int middle_val = arr[j] + outer_val;
            
            // Inner loop - fully contained within middle
            for (int k = 0; k < j; k++) {
                // Multiple basic blocks within innermost loop
                if (middle_val > 1000) {
                    sum += arr[k] * 3;
                } else {
                    sum += arr[k];
                }
                
                // Early exit creates additional basic block
                if (sum > 10000) {
                    break;
                }
            }
            
            // Loop-invariant code to encourage strength reduction
            int stride = 2;  // Made loop-invariant
            if (j % stride == 0) {
                sum += middle_val;
            }
        }
        
        // Another conditional exit point
        if (sum > 50000) {
            break;
        }
    }
    return sum;
}

static int overlapping_loops(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    // Function B: Two loops that share common basic blocks
    // First loop
    for (i = 0; i < n/2; i++) {
        result += arr[i];
        
        // Common block that both loops might reach
        common_block:
        if (result % 2 == 0) {
            result *= 2;
        } else {
            result /= 2;
        }
        
        // Conditional break
        if (result > 1000) {
            goto after_first;
        }
    }
    after_first:
    
    // Second loop - partially overlaps with first via goto
    for (int j = 0; j < n/2; j++) {
        result -= arr[j];
        
        // Jump to common block - creates partial overlap
        if (j == n/4) {
            goto common_block;
        }
        
        // Different exit condition
        if (result < -1000) {
            break;
        }
    }
    
    return result;
}

static int switch_in_loop(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    // Function C: Loop with switch containing break to outside
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        // Switch with break that exits the loop
        switch (state) {
            case 0:
                if (total > 500) {
                    state = 1;
                }
                break;
            case 1:
                if (total > 1000) {
                    // This break exits the switch, not the loop
                    break;
                }
                // Fall through
            case 2:
                // Complex control flow with label
                if (total > 2000) {
                    goto loop_exit;
                }
                break;
            default:
                total *= 2;
        }
        
        // Another basic block in loop body
        if (i == n/2) {
            total -= 100;
        }
    }
    loop_exit:
    
    return total;
}

static int irreducible_flow(int *arr, int n) {
    int acc = 0;
    int i = 0, j = 0;
    
    // Additional function with irreducible control flow
    start_outer:
    while (i < n) {
        acc += arr[i];
        i++;
        
        if (acc > 500) {
            goto inner_loop;
        }
        
        continue_outer:
        if (i % 3 == 0) {
            acc -= 10;
        }
    }
    goto finish;
    
    inner_loop:
    while (j < n/2) {
        acc += arr[j] * 2;
        j++;
        
        if (acc < 0) {
            goto continue_outer;
        }
        
        if (j > n/4) {
            goto start_outer;
        }
    }
    
    finish:
    return acc;
}

int main() {
    // Initialize array with sequential values
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100;  // Keep values moderate
    }
    
    // Call all functions to create various loop structures
    int result1 = triple_nested_loop(arr, N);
    int result2 = overlapping_loops(arr + 100, N);
    int result3 = switch_in_loop(arr + 200, N);
    int result4 = irreducible_flow(arr + 300, N);
    
    // Combine results to prevent elimination
    int final_result = result1 + result2 + result3 + result4;
    
    // Use volatile to ensure computation isn't optimized away
    volatile int sink = final_result;
    
    // Print to ensure side effects
    printf("Result: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}
