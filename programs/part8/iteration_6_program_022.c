#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result_gt[N];
    ALIGNED int result_ge[N];
    ALIGNED int result_lt[N];
    ALIGNED int result_le[N];
    
    // Initialize arrays with varying patterns
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Values 0-99
        b[i] = (i * 7) % 100;      // Different pattern
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (greater than) - covers lines for GT_EXPR transformation
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // Scalar comparison that should be vectorized
    }
    
    // Loop 2: GE_EXPR (greater than or equal) - covers lines for GE_EXPR
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];  // Different comparison type
    }
    
    // Loop 3: LT_EXPR (less than) - covers lines for LT_EXPR
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];  // Another comparison type
    }
    
    // Loop 4: LE_EXPR (less than or equal) - covers lines for LE_EXPR
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];  // Final comparison type
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += result_gt[i] + result_ge[i] + result_lt[i] + result_le[i];
    }
    
    // Print checksum to ensure all comparisons are executed
    printf("Checksum: %d\n", sum);
    
    // Additional test with mixed comparisons in same loop
    ALIGNED int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix of different comparison types in conditional
        if (a[i] > b[i]) {
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {
            mixed_results[i] = 2;
        } else {
            mixed_results[i] = 0;
        }
    }
    
    // Another usage pattern: accumulate based on comparisons
    int count_gt = 0, count_le = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) count_gt++;
        if (a[i] <= b[i]) count_le++;
    }
    
    printf("Count > : %d, Count <= : %d\n", count_gt, count_le);
    
    return sum > 0 ? 0 : 1;  // Return value depends on comparisons
}
