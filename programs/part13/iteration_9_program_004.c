#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define N 32

static int function_a(int *arr, int n) {
    int sum = 0;
    // Triple nested loop - fully contained sub-loops
    for (int i = 0; i < n; i++) {
        // Outer loop with multiple basic blocks
        if (arr[i] < 0) break;
        
        for (int j = 0; j < i; j++) {
            // Middle loop with invariant calculation
            int stride = 2; // Loop invariant
            for (int k = 0; k < j; k++) {
                // Innermost fully contained loop
                sum += arr[k * stride % n];
                if (sum > 1000000) break; // Multiple exit point
            }
            // Additional basic block in middle loop
            sum += j;
        }
        
        // Another basic block in outer loop
        sum += i * 3;
    }
    return sum;
}

static int function_b(int *arr, int n) {
    int result = 0;
    int i = 0;
    
    // First loop with goto to shared block
    while (i < n / 2) {
        result += arr[i];
        i++;
        if (result % 7 == 0) {
            goto shared_block; // Creates overlapping control flow
        }
    }
    
    // Reset for second loop
    i = n / 2;
    
    // Second loop that also jumps to shared block
    while (i < n) {
        result -= arr[i];
        i++;
        if (result % 11 == 0) {
            goto shared_block; // Both loops can reach shared_block
        }
    }
    
    goto end;
    
shared_block:
    // Shared basic block between the two loops
    // This creates partial overlap in block bitmaps
    result *= 2;
    if (result > 1000) {
        return result; // Early return
    }
    
end:
    return result;
}

static int function_c(int *arr, int n) {
    int total = 0;
    int state = 0;
    
    // Loop with switch containing break to label
    for (int i = 0; i < n; i++) {
        total += arr[i];
        
        switch (state) {
            case 0:
                if (total > 500) {
                    state = 1;
                }
                break;
            case 1:
                if (arr[i] % 2 == 0) {
                    goto loop_exit; // break out of loop from switch
                }
                break;
            case 2:
                total *= 2;
                break;
        }
        
        // Loop-invariant code
        int invariant = n * 2;
        if (total > invariant) {
            break; // Another exit point
        }
    }
    
loop_exit:
    // Label outside the loop
    return total;
}

static int function_d(int *arr, int n) {
    int acc = 0;
    int i = 0, j = 0;
    
    // Two interleaved loops with complex control flow
start_outer:
    if (i >= n) goto finish;
    
    // First inner loop structure
    while (j < n && j < i + 5) {
        acc += arr[j] - arr[i];
        j++;
        if (acc < 0) {
            // Jump to different part of outer loop
            i += 2;
            goto start_outer;
        }
    }
    
    // Second inner loop that shares some blocks
    for (int k = 0; k < 3; k++) {
        acc += k * arr[i];
        if (acc % 13 == 0) {
            // This creates overlapping but not contained relationship
            goto partial_shared;
        }
    }
    
    i++;
    j = 0;
    goto start_outer;

partial_shared:
    // Block shared between the for-loop and potential other paths
    acc /= 2;
    i++;
    goto start_outer;

finish:
    return acc;
}

int main() {
    // Initialize array with sequential values
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100; // Prevent overflow in calculations
    }
    
    // Call functions with different slices to create varied loop structures
    int result = 0;
    
    // Function A: Fully contained nested loops
    result += function_a(arr, N);
    
    // Function B: Overlapping loops via goto
    result += function_b(arr + 100, N * 2);
    
    // Function C: Loop with switch and labeled break
    result += function_c(arr + 200, N);
    
    // Function D: Complex interleaved loops
    result += function_d(arr + 300, N);
    
    // Use result to prevent elimination
    printf("Result: %d\n", result);
    
    return result % 256;
}
