#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    int ALIGNED a[N], b[N];
    int ALIGNED gt_results[N], le_results[N];
    int ALIGNED lt_results[N], ge_results[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Varying pattern
        b[i] = (i * 7) % 100;      // Different pattern
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (greater than) and LT_EXPR (less than)
    // This should trigger the transformation for > and < comparisons
    for (int i = 0; i < N; i++) {
        gt_results[i] = a[i] > b[i];   // GT_EXPR
        lt_results[i] = a[i] < b[i];   // LT_EXPR
    }
    
    // Loop 2: GE_EXPR (greater or equal) and LE_EXPR (less or equal)
    // This should trigger the transformation for >= and <= comparisons
    for (int i = 0; i < N; i++) {
        ge_results[i] = a[i] >= b[i];  // GE_EXPR
        le_results[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use results to prevent dead code elimination
    // Also creates data dependencies to ensure vectorization is beneficial
    for (int i = 0; i < N; i++) {
        sum += gt_results[i] - lt_results[i] + ge_results[i] - le_results[i];
    }
    
    // Conditional branch using comparison results
    int threshold = N / 4;
    if (sum > threshold) {
        printf("Sum of comparisons: %d (above threshold)\n", sum);
    } else {
        printf("Sum of comparisons: %d (below threshold)\n", sum);
    }
    
    // Additional pattern: mixed comparisons in same loop
    // This might trigger different transformation paths
    int ALIGNED mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix of different comparison types
        if (a[i] > b[i]) {          // GT_EXPR
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {  // LE_EXPR
            mixed_results[i] = -1;
        } else {
            mixed_results[i] = 0;
        }
    }
    
    // Final usage to prevent optimization
    int final_check = 0;
    for (int i = 0; i < N; i++) {
        final_check += mixed_results[i];
    }
    
    printf("Final check value: %d\n", final_check);
    
    return (sum > 0 && final_check != 0) ? 0 : 1;
}
