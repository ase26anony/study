#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main() {
    // Declare aligned arrays to help vectorization
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED int result_gt[N];
    ALIGNED int result_le[N];
    ALIGNED int result_lt[N];
    ALIGNED int result_ge[N];
    
    // Initialize arrays with distinct values
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      // Pattern: 0, 3, 6, 9, ...
        b[i] = (i * 2) % 100;      // Pattern: 0, 2, 4, 6, ...
    }
    
    int sum = 0;
    
    // Loop 1: GT_EXPR (>)
    // This should trigger the GT_EXPR path in vectorizable_comparison
    for (int i = 0; i < N; i++) {
        result_gt[i] = a[i] > b[i];  // Scalar comparison to be vectorized
    }
    
    // Loop 2: LE_EXPR (<=)
    // This should trigger the LE_EXPR path
    for (int i = 0; i < N; i++) {
        result_le[i] = a[i] <= b[i];
    }
    
    // Loop 3: LT_EXPR (<)
    // This should trigger the LT_EXPR path
    for (int i = 0; i < N; i++) {
        result_lt[i] = a[i] < b[i];
    }
    
    // Loop 4: GE_EXPR (>=)
    // This should trigger the GE_EXPR path
    for (int i = 0; i < N; i++) {
        result_ge[i] = a[i] >= b[i];
    }
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += result_gt[i] + result_le[i] + result_lt[i] + result_ge[i];
    }
    
    // Print checksum to ensure all comparisons are executed
    printf("Checksum: %d\n", sum);
    
    // Additional test with mixed comparisons in same loop
    // This might trigger different transformation patterns
    ALIGNED int mixed_results[N];
    for (int i = 0; i < N; i++) {
        // Mix of different comparison operators
        if (i % 4 == 0) {
            mixed_results[i] = a[i] > b[i];    // GT_EXPR
        } else if (i % 4 == 1) {
            mixed_results[i] = a[i] >= b[i];   // GE_EXPR
        } else if (i % 4 == 2) {
            mixed_results[i] = a[i] < b[i];    // LT_EXPR
        } else {
            mixed_results[i] = a[i] <= b[i];   // LE_EXPR
        }
    }
    
    int mixed_sum = 0;
    for (int i = 0; i < N; i++) {
        mixed_sum += mixed_results[i];
    }
    printf("Mixed checksum: %d\n", mixed_sum);
    
    return (sum > 0 && mixed_sum > 0) ? 0 : 1;
}
