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
        // Middle loop - fully contained in outer
        for (int j = 0; j < MID_SIZE && (i + j) < n; j++) {
            // Inner loop - fully contained in middle
            for (int k = 0; k < INNER_SIZE; k++) {
                sum += arr[i + j] * k;
                // Multiple exit point
                if (sum > 1000000) break;
            }
            // Loop-invariant code
            int stride = 2;
            sum += arr[i + j * stride % n];
        }
        // Another exit point
        if (sum < 0) break;
    }
    
    return sum;
}

static int function_b(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    // Two loops that will share a common basic block via goto
    // First loop
    while (i < n / 2) {
        result += arr[i] * 3;
        i++;
        
        // Conditional break creates multiple basic blocks
        if (result > 50000) {
            goto common_block;
        }
        
        if (i % 7 == 0) {
            continue;
        }
        
        result -= arr[i] / 2;
    }
    
    // Jump to skip the second loop in some cases
    if (result < 0) {
        goto skip_second;
    }
    
common_block:
    // This block is shared between both loops
    result = (result * 2) % 1000;
    
    // Second loop - partially overlaps with first via common_block
    for (int j = n / 2; j < n; j++) {
        result += arr[j] - j;
        
        // Jump back to common block
        if (j % 5 == 0) {
            goto common_block;
        }
        
        // Multiple exit points
        if (result < -1000 || result > 10000) {
            break;
        }
    }
    
skip_second:
    return result;
}

static int function_c(int* arr, int n) {
    int total = 0;
    int state = 0;
    
    // Loop with switch containing break to exit loop
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        // Switch inside loop with complex control flow
        switch (state) {
            case 0:
                if (total % 2 == 0) {
                    state = 1;
                } else {
                    state = 2;
                }
                // Regular break - only breaks from switch
                break;
                
            case 1:
                total += i * 3;
                if (total > 1000) {
                    // This break will exit the entire loop, not just switch
                    goto loop_exit;
                }
                state = 3;
                break;
                
            case 2:
                total -= i * 2;
                state = 0;
                // Fall through
                
            case 3:
                total /= 2;
                state = (state + 1) % 4;
                break;
                
            default:
                // Exit loop from default case
                goto loop_exit;
        }
        
        // Another basic block in the loop
        if (i % 11 == 0) {
            continue;
        }
        
        total = (total * 7) % 10000;
    }
    
loop_exit:
    return total;
}

static int function_d(int* arr, int n) {
    int acc = 0;
    
    // Nested loops with irregular structure
    int outer = 0;
    while (outer < n) {
        // First inner loop
        for (int inner1 = 0; inner1 < 4 && outer + inner1 < n; inner1++) {
            acc += arr[outer + inner1];
            
            // Early exit creates another basic block
            if (acc < -5000) {
                goto finish_d;
            }
        }
        
        outer += 4;
        
        // Second inner loop that partially overlaps
        if (outer < n) {
            int temp = 0;
            do {
                temp += arr[outer - 1];  // Accesses previous block
                acc += temp;
                
                // Complex condition with multiple basic blocks
                if (temp > 100) {
                    acc -= 50;
                } else {
                    acc += 10;
                }
                
                // Loop-invariant calculation
                int invariant = n / 2;
                acc += arr[invariant % n];
                
            } while (++temp < 3 && outer < n);
        }
        
        // Skip some iterations
        if (acc % 13 == 0) {
            outer += 2;
        }
    }
    
finish_d:
    return acc;
}

int main(int argc, char** argv) {
    // Initialize array with sequential values
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i * 3 + 1;
    }
    
    // Call functions with different slices of the array
    int result = 0;
    
    // Function A: Triple nested fully contained loops
    result += function_a(arr, SIZE);
    
    // Function B: Loops with shared basic block via goto
    result += function_b(arr + 100, SIZE - 100);
    
    // Function C: Loop with switch and goto-based exit
    result += function_c(arr + 200, SIZE - 200);
    
    // Function D: While/do-while with partial overlap
    result += function_d(arr + 300, SIZE - 300);
    
    // Use volatile to prevent elimination
    volatile int sink = result;
    
    // Also print to ensure code executes
    printf("Result: %d\n", result);
    
    return result % 256;
}
