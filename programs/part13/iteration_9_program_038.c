#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define INNER_SIZE 8
#define MIDDLE_SIZE 16

static int function_a(int* arr, int n) {
    int sum = 0;
    
    // Triple-nested loop with fully contained sub-loops
    // This should create proper sub-loop relationships
    for (int i = 0; i < n; i++) {
        // Outer loop body
        int outer_val = arr[i];
        
        // Middle loop - fully contained within outer
        for (int j = 0; j < MIDDLE_SIZE; j++) {
            int middle_val = outer_val + j;
            
            // Inner loop - fully contained within middle
            for (int k = 0; k < INNER_SIZE; k++) {
                // Multiple basic blocks within innermost loop
                if (k % 2 == 0) {
                    sum += middle_val * k;
                } else {
                    sum -= middle_val * k;
                }
                
                // Additional exit point
                if (sum > 1000000) {
                    break;
                }
            }
            
            // Loop-invariant code to encourage strength reduction
            int stride = 3; // Made loop-invariant
            if (j % stride == 0) {
                sum += arr[j] * 2;
            }
        }
        
        // Another conditional break in outer loop
        if (sum < -500000) {
            break;
        }
    }
    
    return sum;
}

static int function_b(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    // Two loops that will share basic blocks via goto
    // This creates overlapping but not subset relationships
    
    // First loop
    while (i < n / 2) {
        result += arr[i] * 2;
        i++;
        
        // Jump to shared code block
        if (i % 3 == 0) {
            goto shared_block;
        }
        
        continue;
        
    shared_block:
        // Shared basic block between two loops
        result -= 5;
        // This creates overlap in block bitmaps
    }
    
    // Reset for second loop
    int j = n / 2;
    
    // Second loop that also uses the shared block
    while (j < n) {
        result += arr[j] * 3;
        j++;
        
        // Jump to same shared block
        if (j % 4 == 0) {
            goto shared_block;
        }
    }
    
    return result;
}

static int function_c(int* arr, int n) {
    int total = 0;
    int i = 0;
    
    // Loop with switch statement containing break to exit loop
    while (i < n) {
        total += arr[i];
        
        // Complex switch inside loop
        switch (i % 5) {
            case 0:
                total += 1;
                break;  // This break is for switch, not loop
            case 1:
                total += 2;
                // Fall through
            case 2:
                total += 3;
                // Use goto to create additional control flow
                if (total > 1000) {
                    goto exit_loop;
                }
                break;
            case 3:
                total += 4;
                // Direct break from loop from within switch
                if (total > 2000) {
                    i = n; // Force exit
                    break;
                }
                break;
            case 4:
                total += 5;
                // Multiple basic blocks
                if (total % 2 == 0) {
                    total *= 2;
                } else {
                    total /= 2;
                }
                break;
        }
        
        i++;
        continue;
        
    exit_loop:
        // Label target for goto from switch
        break; // This breaks the while loop
    }
    
    return total;
}

static int function_d(int* arr, int n) {
    // Another pattern: nested loops with partial overlap
    int sum1 = 0, sum2 = 0;
    
    // First loop nest
    for (int i = 0; i < n; i += 2) {
        for (int j = i; j < n && j < i + 4; j++) {
            sum1 += arr[j];
            
            // Conditional that creates additional basic block
            if (sum1 > 500) {
                sum1 -= 250;
            }
        }
    }
    
    // Second loop nest that overlaps partially
    for (int i = 1; i < n; i += 2) {
        // This inner loop shares some but not all blocks with previous
        for (int j = i; j < n && j < i + 3; j++) {
            sum2 += arr[j] * 2;
            
            // Different condition structure
            if (sum2 % 100 == 0) {
                sum2 += 50;
            } else {
                sum2 -= 25;
            }
        }
    }
    
    return sum1 + sum2;
}

int main() {
    // Initialize array with sequential values
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100; // Keep values reasonable
    }
    
    // Call functions with different slices to create varied loop structures
    int result = 0;
    
    // Use volatile to prevent elimination
    volatile int force_keep;
    
    result += function_a(arr, SIZE / 4);
    force_keep = result; // Prevent dead code elimination
    
    result += function_b(arr + SIZE / 4, SIZE / 4);
    force_keep = result;
    
    result += function_c(arr + SIZE / 2, SIZE / 4);
    force_keep = result;
    
    result += function_d(arr + 3 * SIZE / 4, SIZE / 4);
    force_keep = result;
    
    // Print result to ensure code isn't eliminated
    printf("Result: %d\n", result);
    
    return result % 256; // Return non-zero to indicate execution
}
