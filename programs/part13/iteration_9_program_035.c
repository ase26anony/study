#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define N 32

static int function_a(int* arr, int n) {
    int sum = 0;
    // Triple nested loop - fully contained sub-loops
    for (int i = 0; i < n; i++) {
        // Outer loop body
        int temp = arr[i];
        for (int j = 0; j < i; j++) {
            // Middle loop - fully contained in outer
            for (int k = 0; k < j; k++) {
                // Inner loop - fully contained in middle
                sum += temp * k;
                if (sum > 1000000) break; // Multiple exit point
            }
            temp += j;
            if (temp > 1000) break; // Another exit point
        }
        // Loop-invariant code (j is not available here)
        int stride = 3; // Loop invariant
        sum += arr[i * stride % n];
    }
    return sum;
}

static int function_b(int* arr, int n) {
    int result = 0;
    int i = 0;
    
    // First loop with goto to create overlapping control flow
loop1:
    while (i < n) {
        result += arr[i];
        i++;
        if (i % 7 == 0) {
            goto common_block; // Jump to common block
        }
        if (result > 50000) break;
    }
    
    i = n / 2;
    // Second loop that also uses the common block
    while (i < n) {
        result -= arr[i];
        i++;
        if (i % 5 == 0) {
            goto common_block; // Same common block
        }
    }
    
    return result;

common_block:
    // Common basic block shared by both loops
    result *= 2;
    // Jump back to appropriate location
    if (i < n / 2) {
        goto loop1;
    }
    return result;
}

static int function_c(int* arr, int n) {
    int total = 0;
    int i = 0;
    
    // Loop with switch containing break to outside
    while (i < n) {
        total += arr[i];
        
        switch (i % 4) {
            case 0:
                total += 1;
                break;
            case 1:
                total += 2;
                break;
            case 2:
                total += 3;
                // This break exits the switch, not the loop
                break;
            case 3:
                total += 4;
                if (total > 10000) {
                    goto loop_exit; // Exit loop from switch
                }
                break;
        }
        
        // Multiple basic blocks within loop
        if (total < 0) {
            total = -total;
        } else if (total > 5000) {
            total /= 2;
        }
        
        i++;
        continue;
        
    loop_exit:
        // Label inside loop but break jumps here
        break;
    }
    
    return total;
}

static int function_d(int* arr, int n) {
    // Adjacent loops that might be analyzed together
    int sum1 = 0, sum2 = 0;
    
    // First adjacent loop
    for (int i = 0; i < n/2; i++) {
        sum1 += arr[i];
        // Complex exit condition
        if (sum1 > 10000 && i > 10) {
            break;
        }
    }
    
    // Second adjacent loop - shares no blocks with first
    for (int j = n/2; j < n; j++) {
        sum2 += arr[j] * 2;
        // Different exit condition
        if (sum2 < -5000 || j > n - 5) {
            break;
        }
    }
    
    // Third loop that uses both sums
    int final = 0;
    for (int k = 0; k < n; k += 3) {
        final += arr[k] + sum1 - sum2;
        // Loop-invariant computation
        int invariant = n * 2; // Invariant
        final %= invariant;
    }
    
    return final;
}

int main() {
    // Initialize array with sequential values
    int arr[SIZE];
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i % 100; // Prevent overflow in computations
    }
    
    // Call functions with different slices to create varied loop structures
    int result = 0;
    
    // Function A: Triple nested fully contained loops
    result += function_a(arr, N);
    
    // Function B: Overlapping loops with goto
    result += function_b(arr + 100, N * 2);
    
    // Function C: Loop with switch and break to label
    result += function_c(arr + 200, N);
    
    // Function D: Adjacent loops
    result += function_d(arr + 300, N * 3);
    
    // Use volatile to prevent elimination
    volatile int sink = result;
    
    // Also print to ensure code runs
    printf("Result: %d\n", result);
    
    return result % 256;
}
